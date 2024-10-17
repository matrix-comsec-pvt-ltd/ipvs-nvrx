//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		BackupManager.c
@brief      The backup manager module provides api for start the manual backup. In case of schedule
            backup it will runs schedule for every one hour and checks whether its time to start the
            schedule backup or stops the schedule backup. The individual backup was performed in its
            separate thread. While taking backup it will uses Disk Manager module api for file IO. If
            backup on FTP, then uploading was done in Disk Manager Module. It will stopped the all
            type of backup if source disk starts formatting.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/resource.h>

/* Application Includes */
#include "Utils.h"
#include "SysTimer.h"
#include "BackupManager.h"
#include "DebugLog.h"
#include "FtpClient.h"
#include "DiskController.h"
#include "EventHandler.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define NO_BACKUP_DISK_TIME             12		// Second
#define SEC_24HOUR                      (HOUR_IN_ONE_DAY * SEC_IN_ONE_HOUR)
#define SEC_WEEK                        (MAX_WEEK_DAYS * SEC_24HOUR)

#define	BACKUP_STATUS_FILE              CONFIG_DIR_PATH "/BackupStatus"
#define BACKUP_END_PERCENT              99
#define AVI_BACKUP_END_DETAIL           "100"

/* Use Default Stack Size*/
#define RUN_MANUALBKP_THREAD_STACK_SZ   (0 * MEGA_BYTE)
#define RUN_SCHEDULEBKP_THREAD_STACK_SZ (0 * MEGA_BYTE)
#define BKP_REC_THREAD_STACK_SZ         (2 * MEGA_BYTE)
#define CLIP_BKP_REC_THREAD_STACK_SZ    (2 * MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    BACKUP_FAIL_DISK_FULL,
    BACKUP_FAIL_NO_DISK,
    BACKUP_FAIL_DISK_FAULT,
    BACKUP_FAIL_SOURCE_ERROR,
    BACKUP_FAIL_COPY_ERROR,
    MAX_BACKUP_FAIL_REASON

}BACKUP_FAIL_REASON_e;

typedef enum
{
    BACKUP_PREVIOUS_HOUR,
    BACKUP_CURRENT_HOUR,
    BACKUP_STOP_HOUR,
    MAX_BACKUP_STATUS_HOUR

}PREVIOUS_HLT_STATUS_e;

typedef	struct
{
    time_t					startPoint; // from where back up should be started
    time_t					stopPoint;  // from where back up should be stopped
    BACKUP_HLT_STATUS_e		status;     // status of backup for health

}BACKUP_STATUS_t;

// This will gives the information backup which is currently active
typedef struct
{
    SB_LOCATION_e			location;               // where backup should taken
    BOOL					runFlg;                 // Terminate Flag
    BOOL					threadTerFlg;           // Thread Terminate Flag
    BOOL					isBackupStopdOnUserReq; // Stop Backup on User Request Flag
    time_t					startPoint;             // from where back up should be started
    time_t					stopPoint;              // from where back up should be stoped
	pthread_mutex_t			dataMutex;
	pthread_mutex_t 		condMutex;
	pthread_cond_t 			condSignal;
    BACKUP_STATUS_t			lastBackupStatus;       // Last Backup Status
    BACKUP_DISK_STATUS_e	backupStatus;           // Backup Status

}BACKUP_INFO_t;

typedef struct
{
	UINT32					noOfRecords;
	BACKUP_FORMAT_e			format;
	SEARCH_RESULT_t			records[MAX_MANUAL_BACK_RCD];
	UINT64					recordSize[MAX_MANUAL_BACK_RCD];
	UINT64					totalSize;
	UINT64					processedSize;
	NW_CMD_BKUP_RCD_CB 		callback;
	INT32					connFd;
	CHAR					userName[MAX_USER_ACCOUNT_USERNAME_WIDTH];
	RECORD_ON_DISK_e		recStorageDrive;
    MB_LOCATION_e			backupLocation;
    CLIENT_CB_TYPE_e        clientCbType;

}BKUP_RCD_PARAM_t;

typedef struct
{
	UINT32					noOfRecords;
	BACKUP_FORMAT_e			format;
	SYNC_CLIP_DATA_t		records[MAX_MANUAL_BACK_RCD];
	UINT64					recordSize[MAX_MANUAL_BACK_RCD];
	UINT64					totalSize;
	UINT64					processedSize;
	NW_CMD_BKUP_RCD_CB 		callback;
	INT32					connFd;
	CHAR					userName[MAX_USER_ACCOUNT_USERNAME_WIDTH];
	RECORD_ON_DISK_e		recStorageDrive;
    CLIENT_CB_TYPE_e        clientCbType;

}BKUP_SYNC_CLIP_PARAM_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static BACKUP_INFO_t 			backupInfo[DM_MAX_BACKUP];
static TIMER_HANDLE				backupNodiskTmrHandle;
static BACKUP_FAIL_REASON_e		backUpFailReasonStatus = MAX_BACKUP_FAIL_REASON;
static PREVIOUS_HLT_STATUS_e	prevHealthStatus = BACKUP_PREVIOUS_HOUR;

static const UINT8 sbEveryHourModeCnt[MAX_SB_EVERY_HOUR_MODE] = {1, 2, 3, 4, 6};

static const CHARPTR backupStatusStr[BACKUP_HLT_STATUS_MAX] =
{
    "Stop",
    "Running",
    "Complete",
    "Incomplete",
    "Disable"
};

static const CHARPTR backupNameStr[DM_MAX_BACKUP] = {"Manual", "Schedule"};

static const CHARPTR backupFailStr[MAX_BACKUP_FAIL_REASON] =
{
    "Backup Device Full",
    "Backup Device Unavailable",
    "Backup Device Error",
    "Source Error",
    "Data Transfer Error"
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void backupNodiskCallback(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL backupScheduler(UINT32 count);
//-------------------------------------------------------------------------------------------------
static VOIDPTR runManualBackup(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static BOOL abortBackup(UINT32 userData);
//-------------------------------------------------------------------------------------------------
static void checkScheduleBackup(void);
//-------------------------------------------------------------------------------------------------
static VOIDPTR runScheduleBackup(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static void writeBackupEvent(time_t startTime, time_t endTime, UINT8 eventInfo, DM_BACKUP_TYPE_e backupType, CHARPTR advaceDetails);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e stopBackup(DM_BACKUP_TYPE_e backupType);
//-------------------------------------------------------------------------------------------------
static BOOL setBackupStatusFile(DM_BACKUP_TYPE_e backupType);
//-------------------------------------------------------------------------------------------------
static BOOL getBackupStatusFile(DM_BACKUP_TYPE_e backupType);
//-------------------------------------------------------------------------------------------------
static void terminateAllBackup(void);
//-------------------------------------------------------------------------------------------------
static BOOL backUpRecordEventCb(VOIDPTR privData, UINT64 dataSize,BOOL checkStopFlag);
//-------------------------------------------------------------------------------------------------
static BOOL backUpSyncRecordEventCb(VOIDPTR privData, UINT64 dataSize,BOOL checkStopFlag);
//-------------------------------------------------------------------------------------------------
static VOIDPTR backUpRecordThread(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR backUpClipRecordThread(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes backup manager, it will start a timer which will runs periodically
 *          and check whether its time to schedule back and then intiates schedule back up.
 */
void InitBackupManager(void)
{
	DM_BACKUP_TYPE_e			backupType;
	ONE_MIN_NOTIFY_t			oneMinFun;
	MANUAL_BACKUP_CONFIG_t 		manualBackupCfg;
	SCHEDULE_BACKUP_CONFIG_t	scheduleBackupCfg;

	ReadManualBackupConfig(&manualBackupCfg);
	ReadScheduleBackupConfig(&scheduleBackupCfg);

	oneMinFun.funcPtr = backupScheduler;
	oneMinFun.userData = 0;

	RegisterOnMinFun(&oneMinFun);
	backupNodiskTmrHandle = INVALID_TIMER_HANDLE;

	for(backupType = 0; backupType < DM_MAX_BACKUP; backupType++)
	{
		backupInfo[backupType].runFlg = STOP;
		backupInfo[backupType].threadTerFlg = FALSE;
		backupInfo[backupType].isBackupStopdOnUserReq = FALSE;
		backupInfo[backupType].location = SB_TO_USB_DEVICE;
		backupInfo[backupType].startPoint = 0;
		backupInfo[backupType].stopPoint = 0;
        MUTEX_INIT(backupInfo[backupType].dataMutex, NULL);
        MUTEX_INIT(backupInfo[backupType].condMutex, NULL);
		pthread_cond_init(&backupInfo[backupType].condSignal, NULL);
	}

    backupInfo[DM_MANUAL_BACKUP].backupStatus = (manualBackupCfg.manualBackup == ENABLE) ? BACKUP_READY : BACKUP_DISABLE;

    if((scheduleBackupCfg.scheduleBackup == ENABLE) && (scheduleBackupCfg.backupLocation == SB_TO_USB_DEVICE))
	{
		backupInfo[DM_SCHEDULE_BACKUP].backupStatus = BACKUP_READY;
	}
	else
	{
		backupInfo[DM_SCHEDULE_BACKUP].backupStatus = BACKUP_DISABLE;
	}

	//Get last backup Status
	getBackupStatusFile(DM_SCHEDULE_BACKUP);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function de initialize all the resources of back up manager module.
 */
void DeInitBackupManager(void)
{
	terminateAllBackup();
    DPRINT(BACKUP_MANAGER, "backup manager de-initialize");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates manual back up configuration.
 * @param   newCfg
 */
void ManualBackupCfgUpdate(MANUAL_BACKUP_CONFIG_t newCfg)
{
    if (StopManualBackup() != CMD_BACKUP_NOT_IN_PROG)
	{
        return;
    }

    MUTEX_LOCK(backupInfo[DM_MANUAL_BACKUP].dataMutex);
    backupInfo[DM_MANUAL_BACKUP].backupStatus = (newCfg.manualBackup == DISABLE) ? BACKUP_DISABLE : BACKUP_READY;
    MUTEX_UNLOCK(backupInfo[DM_MANUAL_BACKUP].dataMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates Schedule back up configuration
 * @param   newCfg
 * @param   oldCfg
 */
void ScheduleBackupCfgUpdate(SCHEDULE_BACKUP_CONFIG_t *newCfg, SCHEDULE_BACKUP_CONFIG_t *oldCfg)
{
	NET_CMD_STATUS_e		retVal;
	BACKUP_DISK_STATUS_e	backupState;

	retVal = StopScheduleBackup();
	if(retVal == CMD_BACKUP_NOT_IN_PROG)
	{
        if((newCfg->scheduleBackup == ENABLE) && (newCfg->backupLocation == SB_TO_USB_DEVICE))
		{
			backupState = BACKUP_READY;
		}
		else
		{
			backupState = BACKUP_DISABLE;
		}

        MUTEX_LOCK(backupInfo[DM_SCHEDULE_BACKUP].dataMutex);
		backupInfo[DM_SCHEDULE_BACKUP].backupStatus = backupState;
        MUTEX_UNLOCK(backupInfo[DM_SCHEDULE_BACKUP].dataMutex);
	}

	if (newCfg->backupLocation != oldCfg->backupLocation)
	{
		StopBackUpCleanUp();
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates backup status upon detection of any USB disk;
 * @param   diskType
 */
void UpdateBackupStatus(DM_BACKUP_TYPE_e diskType)
{
	BACKUP_DISK_STATUS_e		backupState = BACKUP_DISABLE;
	MANUAL_BACKUP_CONFIG_t		manBckCfg;
	SCHEDULE_BACKUP_CONFIG_t	schdlBckCfg;

	if(diskType == DM_SCHEDULE_BACKUP)
	{
		ReadScheduleBackupConfig(&schdlBckCfg);
        if((schdlBckCfg.scheduleBackup == ENABLE) && (schdlBckCfg.backupLocation == SB_TO_USB_DEVICE))
		{
			backupState = BACKUP_READY;
		}
	}
	else
	{
		ReadManualBackupConfig(&manBckCfg);
		if(manBckCfg.manualBackup == ENABLE)
		{
			backupState = BACKUP_READY;
		}
	}

    MUTEX_LOCK(backupInfo[diskType].dataMutex);
	backupInfo[diskType].backupStatus = backupState;
    MUTEX_UNLOCK(backupInfo[diskType].dataMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function starts manual backup if manual back was enable and proper USB disk was
 *          present. It will create a thread which will gives backup to given device using disk
 *          manager's file IO function.
 * @param   startTime
 * @param   endTime
 * @return  Status
 */
NET_CMD_STATUS_e StartManualBackup(struct tm startTime, struct tm endTime)
{
	NET_CMD_STATUS_e		retVal = CMD_SUCCESS;
    UINT64					totalSize = 0;
    float					freeSize = 0;
	USB_DISK_INFO_t			backupDiskInfo;
    time_t					localTime;
    time_t                  startPoint, stopPoint;
	struct tm 				brokenTime;
	TIMER_INFO_t			timerInfo;
    DM_BACKUP_TYPE_e		backupType = DM_MANUAL_BACKUP;//assign backup type
	MANUAL_BACKUP_CONFIG_t 	manualBackupCfg;
	DISK_VOLUME_INFO_t 	 	nddVolInfo;
	NETWORK_DRIVE_CONFIG_t	nwConfig;

	ReadManualBackupConfig(&manualBackupCfg);

    /* Check if backup is enabled for any camera */
    if ((manualBackupCfg.manualBackup == DISABLE) || (TRUE == IS_ALL_CAMERA_MASK_BIT_CLR(manualBackupCfg.camera)))
	{
        retVal = CMD_PROCESS_ERROR;
    }
    else if (manualBackupCfg.backupLocation == MB_TO_USB_DEVICE)
    {
        GetBackupDiskPara(backupType, &backupDiskInfo);
        if (backupDiskInfo.status == BACKUP_DISK_FORMATTING)
        {
            return CMD_FORMAT_IN_PROCESS;
        }

        if ((backupDiskInfo.status == BACKUP_DISK_UNPLUGGING) || (backupDiskInfo.status == BACKUP_DISK_FAULT))
        {
            return CMD_NO_DISK_FOUND;
        }

        if (backupDiskInfo.status == BACKUP_NO_DISK)
        {
            retVal = CMD_NO_DISK_FOUND;
        }
    }
    else
    {
        ReadSingleNetworkDriveConfig((manualBackupCfg.backupLocation - MB_TO_NETWORK_DRIVE_1), &nwConfig);
        if(nwConfig.enable == DISABLE)
        {
            return CMD_NO_DISK_FOUND;
        }

        if(SUCCESS != GetNddVolumeInfo((manualBackupCfg.backupLocation - MB_TO_NETWORK_DRIVE_1), &nddVolInfo))
        {
            WPRINT(BACKUP_MANAGER, "ndd disabled: [ndd=%d]", manualBackupCfg.backupLocation - MB_TO_NETWORK_DRIVE_1);
            return CMD_NO_DISK_FOUND;
        }

        if (nddVolInfo.status == DM_DISK_VOL_FORMATTING)
        {
            return CMD_FORMAT_IN_PROCESS;
        }
        else if (nddVolInfo.status == DM_DISK_VOL_FULL)
        {
            return CMD_REC_MEDIA_FULL;
        }

        if ((nddVolInfo.status == DM_DISK_VOL_CLEANUP) || (nddVolInfo.status == DM_DISK_VOL_READ_ONLY))
        {
            /* For read only status, also use CMD_NO_DISK_FOUND */
            return CMD_NO_DISK_FOUND;
        }

        if ((nddVolInfo.status == DM_DISK_NOT_CONNECTED) || (nddVolInfo.status == DM_DISK_VOL_FAULT)
                || (nddVolInfo.status == DM_DISK_VOL_INCOMP_VOLUME))
        {
            retVal = CMD_NO_DISK_FOUND;
        }
    }

    if (retVal != CMD_SUCCESS)
    {
        if (backupNodiskTmrHandle == INVALID_TIMER_HANDLE)
        {
            SetBackupLedState(NO_BACKUP_DEVICE_FOUND);
            timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(NO_BACKUP_DISK_TIME);
            timerInfo.data = 0;
            timerInfo.funcPtr = backupNodiskCallback;
            StartTimer(timerInfo, &backupNodiskTmrHandle);
        }
        else
        {
            ReloadTimer(backupNodiskTmrHandle, CONVERT_SEC_TO_TIMER_COUNT(NO_BACKUP_DISK_TIME));
        }

        /* Generate Log of manual backup was fail */
        if (retVal == CMD_PROCESS_ERROR)
        {
            EPRINT(BACKUP_MANAGER, "manual backup not configured");
        }
        else
        {
            EPRINT(BACKUP_MANAGER, "manual backup disk not present");
            writeBackupEvent(backupInfo[backupType].startPoint, backupInfo[backupType].stopPoint,
                             EVENT_INCOMPLETE, backupType, backupFailStr[BACKUP_FAIL_NO_DISK]);
        }

        return retVal;
    }

    MUTEX_LOCK(backupInfo[backupType].dataMutex);
    if (backupInfo[backupType].runFlg != STOP)
    {
        MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
        return CMD_BACKUP_IN_PROCESS;
    }

    backupInfo[backupType].runFlg = START;
    backupInfo[backupType].threadTerFlg = FALSE;
    DeleteTimer(&backupNodiskTmrHandle);
    GetLocalTimeInSec(&localTime);

    if ((startTime.tm_hour == 0) && (startTime.tm_min == 0) && (startTime.tm_sec == 0) && (startTime.tm_year == 0)
            && (startTime.tm_mday == 0) && ((startTime.tm_mon+1) == 0)
            && (endTime.tm_hour == 0) && (endTime.tm_min == 0) && (endTime.tm_sec == 0) && (endTime.tm_year == 0)
            && (endTime.tm_mday == 0) && ((endTime.tm_mon+1) == 0))
    {
        DPRINT(BACKUP_MANAGER, "manual backup is enabled with valid backup location");
        if(manualBackupCfg.duration <= MB_LAST_24_HOUR)
        {
            backupInfo[backupType].startPoint = (localTime - (SEC_IN_ONE_HOUR *(manualBackupCfg.duration + 1)));
            backupInfo[backupType].stopPoint = localTime;
        }
        else if(manualBackupCfg.duration == MB_PREVIOUS_DAY)
        {
            ConvertLocalTimeInBrokenTm(&localTime, &brokenTime);
            brokenTime.tm_hour = 0;
            brokenTime.tm_min = 0;
            brokenTime.tm_sec = 0;
            ConvertLocalTimeInSec(&brokenTime, &localTime);
            backupInfo[backupType].startPoint = (localTime - SEC_24HOUR);
            backupInfo[backupType].stopPoint = localTime;
        }
        else if(manualBackupCfg.duration == MB_LAST_WEEK)
        {
            ConvertLocalTimeInBrokenTm(&localTime, &brokenTime);
            brokenTime.tm_hour = 0;
            brokenTime.tm_min = 0;
            brokenTime.tm_sec = 0;
            ConvertLocalTimeInSec(&brokenTime, &localTime);
            backupInfo[backupType].startPoint = (localTime - SEC_WEEK);
            backupInfo[backupType].stopPoint = localTime;
        }
        else
        {
            backupInfo[backupType].stopPoint = localTime;
            RESET_EXTRA_BORKEN_TIME_VAR(brokenTime);
            brokenTime.tm_mday = 1;			/* 1st */
            brokenTime.tm_mon = 0;			/* January */
            brokenTime.tm_year = 2020;
            brokenTime.tm_hour = 0;
            brokenTime.tm_min = 0;
            brokenTime.tm_sec = 0;
            ConvertLocalTimeInSec(&brokenTime, &localTime);
            backupInfo[backupType].startPoint = localTime;
        }
    }
    else if(manualBackupCfg.duration == MB_CUSTOM)
    {
        ConvertLocalTimeInSec(&startTime, &localTime);
        backupInfo[backupType].startPoint = localTime;
        ConvertLocalTimeInSec(&endTime, &localTime);
        backupInfo[backupType].stopPoint = localTime;
    }

    startPoint = backupInfo[backupType].startPoint;
    stopPoint = backupInfo[backupType].stopPoint;
    MUTEX_UNLOCK(backupInfo[backupType].dataMutex);

    /* Get Size of Back-Up data */
    GetSizeOfDuration(startPoint, stopPoint, manualBackupCfg.camera, &totalSize);
    if(manualBackupCfg.backupLocation == MB_TO_USB_DEVICE)
    {
        /* Get USB Disk free size */
        sscanf(backupDiskInfo.freeSize, "%f", &freeSize);
    }
    else
    {
        /* Get NDD Disk free size */
        sscanf(nddVolInfo.freeSize, "%f", &freeSize);
    }

    DPRINT(BACKUP_MANAGER, "backup info: [recordSize=%f GB], [freeSize=%f GB]", ((float)(totalSize/GIGA_BYTE)), freeSize);

    /* Check if there's sufficient space in backup media */
    if (freeSize < ((float)(totalSize/GIGA_BYTE)))
    {
        retVal = CMD_INVALID_FILE_SIZE;
        EPRINT(BACKUP_MANAGER, "there's no enough space available in disk");
        writeBackupEvent(startPoint, stopPoint, EVENT_INCOMPLETE, backupType, backupFailStr[BACKUP_FAIL_DISK_FULL]);
    }
    else if (totalSize == 0)
    {
        writeBackupEvent(startPoint, stopPoint, EVENT_INCOMPLETE, backupType, "No records found");
        DPRINT(BACKUP_MANAGER, "no data to take manual backup");
    }
    else
    {
        // create a thread which will starts the actual back up.
        if (SUCCESS == Utils_CreateThread(NULL, runManualBackup, &backupInfo[backupType], DETACHED_THREAD, RUN_MANUALBKP_THREAD_STACK_SZ))
        {
            DPRINT(BACKUP_MANAGER, "manual backup started: [duration=%d], [startTime=%02d/%02d/%04d %02d:%02d:%02d], [startTime=%02d/%02d/%04d %02d:%02d:%02d]",
                   manualBackupCfg.duration,
                   startTime.tm_mday, startTime.tm_mon + 1, startTime.tm_year, startTime.tm_hour, startTime.tm_min, startTime.tm_sec,
                   endTime.tm_mday, endTime.tm_mon + 1, endTime.tm_year, endTime.tm_hour, endTime.tm_min, endTime.tm_sec);
            return CMD_SUCCESS;
        }

        /* Fail to create the thread */
        retVal = CMD_PROCESS_ERROR;
    }

    MUTEX_LOCK(backupInfo[backupType].dataMutex);
    backupInfo[backupType].runFlg = FALSE;
    backupInfo[backupType].threadTerFlg = TRUE;
    MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
    SetBackupLedState((retVal == CMD_INVALID_FILE_SIZE) ? BACKUP_DEVICE_NOT_ENOUGH_SPACE : BACKUP_NO_ACITIVITY);
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is timer callback to give signal
 * @param   data
 */
static void backupNodiskCallback(UINT32 data)
{
    USB_DISK_INFO_t	backupDiskInfo;

	GetBackupDiskPara(DM_MANUAL_BACKUP, &backupDiskInfo);
	if(backupDiskInfo.status == BACKUP_NO_DISK)
	{
		SetBackupLedState(BACKUP_NOT_GOING_ON);
	}
	DeleteTimer(&backupNodiskTmrHandle);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops manual backup if it was running. Because here user wants to stop the manual backup
 * @return  Status
 */
NET_CMD_STATUS_e StopManualBackup(void)
{
	return stopBackup(DM_MANUAL_BACKUP);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops schedule backup if it was running. Because here user wants to stop the manual backup
 * @return  Status
 */
NET_CMD_STATUS_e StopScheduleBackup(void)
{
	return stopBackup(DM_SCHEDULE_BACKUP);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check schedule backup is enabled or not and if enabled and valid config found then
 *          start schedule backup.
 */
static void checkScheduleBackup(void)
{
    time_t						localTime, secDiff;
    time_t                      startPoint, stopPoint;
    struct tm					brokenTime;
    DM_BACKUP_TYPE_e			backupType = DM_SCHEDULE_BACKUP;
    SCHEDULE_BACKUP_CONFIG_t	schdlBackup;

    /* Read the schedule back up configuration */
    ReadScheduleBackupConfig(&schdlBackup);

    if ((schdlBackup.scheduleBackup == DISABLE) || (TRUE == IS_ALL_CAMERA_MASK_BIT_CLR(schdlBackup.camera)) || (schdlBackup.backupLocation >= MAX_SB_LOCATION))
    {
        MUTEX_LOCK(backupInfo[backupType].dataMutex);
        backupInfo[backupType].lastBackupStatus.status = BACKUP_HLT_DISABLE;
        MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
        return;
    }

    if (schdlBackup.backupLocation == SB_TO_USB_DEVICE)
    {
        USB_DISK_INFO_t backupDiskInfo;

        GetBackupDiskPara(backupType, &backupDiskInfo);
        if ((backupDiskInfo.status == BACKUP_DISK_FORMATTING) || (backupDiskInfo.status == BACKUP_DISK_FAULT)
                || (backupDiskInfo.status == BACKUP_DISK_UNPLUGGING))
        {
            MUTEX_LOCK(backupInfo[backupType].dataMutex);
            backupInfo[backupType].lastBackupStatus.status = BACKUP_HLT_DISABLE;
            MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
            return;
        }
    }

    if (SUCCESS != GetLocalTimeInBrokenTm(&brokenTime))
    {
        EPRINT(BACKUP_MANAGER, "fail to get local time for schedule backup");
        return;
    }

    DPRINT(BACKUP_MANAGER, "schedule backup enabled: [mode=%d], [location=%d]", schdlBackup.mode, schdlBackup.backupLocation);
    switch(schdlBackup.mode)
    {
        default:
        case SB_EVERY_HOUR:
        {
            brokenTime.tm_hour = ((brokenTime.tm_hour / sbEveryHourModeCnt[schdlBackup.everyHourMode]) * sbEveryHourModeCnt[schdlBackup.everyHourMode]);
            brokenTime.tm_min = 0;
            brokenTime.tm_sec = 0;
            ConvertLocalTimeInSec(&brokenTime, &localTime);

            startPoint = (localTime - (SEC_IN_ONE_HOUR * sbEveryHourModeCnt[schdlBackup.everyHourMode]));
            stopPoint = localTime;
        }
        break;

        case SB_EVERYDAY:
        {
            // It will take backup of previous of previous day to previous day
            secDiff = (brokenTime.tm_hour < (INT32)schdlBackup.everydayBackup.hour) ? SEC_24HOUR : 0;
            brokenTime.tm_hour = schdlBackup.everydayBackup.hour;
            brokenTime.tm_min = 0;
            brokenTime.tm_sec = 0;
            ConvertLocalTimeInSec(&brokenTime, &localTime);
            localTime -= secDiff;

            startPoint = (localTime - SEC_24HOUR);
            stopPoint = localTime;
        }
        break;

        case SB_WEEKLY:
        {
            // Check if the current day and time is after the scheduled backup time
            if ((brokenTime.tm_wday > (INT32)schdlBackup.weeklyBackup.weekday)
                 || ((brokenTime.tm_wday == (INT32)schdlBackup.weeklyBackup.weekday) && (brokenTime.tm_hour >= (INT32)schdlBackup.weeklyBackup.hour)))
            {
                // If today is after the scheduled day or it's the same day but after the scheduled time
                secDiff = (((time_t)brokenTime.tm_wday - schdlBackup.weeklyBackup.weekday) * SEC_24HOUR);
            }
            else
            {
                // Calculate time difference from previous backup point to current day
                secDiff = ((MAX_WEEK_DAYS - ((time_t)schdlBackup.weeklyBackup.weekday - brokenTime.tm_wday)) * SEC_24HOUR);
            }

            // Get the current time till last hour
            brokenTime.tm_hour = schdlBackup.weeklyBackup.hour;
            brokenTime.tm_min = 0;
            brokenTime.tm_sec = 0;
            ConvertLocalTimeInSec(&brokenTime, &localTime);

            // Subtract the time difference (secDiff) from current time to previous backup time
            localTime -= secDiff;

            // The backup should start from previous of previous backup point to previous backup point
            startPoint = (localTime - SEC_WEEK);
            stopPoint = localTime;
        }
        break;
    }

    MUTEX_LOCK(backupInfo[backupType].dataMutex);
    backupInfo[backupType].startPoint = startPoint;
    backupInfo[backupType].stopPoint = stopPoint;
    if ((backupInfo[backupType].lastBackupStatus.startPoint == startPoint) && (backupInfo[backupType].lastBackupStatus.stopPoint == stopPoint)
        && (backupInfo[backupType].lastBackupStatus.status == BACKUP_HLT_COMPLETE))
    {
        DPRINT(BACKUP_MANAGER, "schedule backup is completed/stop: [lastBackupStatus=%d]", backupInfo[backupType].lastBackupStatus.status);
        MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
        return;
    }

    /* Last backup was halt by user */
    if(backupInfo[backupType].isBackupStopdOnUserReq)
    {
        /* Check if the time slot is same in which user has stopped the backup then do not start the backup again */
        if((backupInfo[backupType].lastBackupStatus.startPoint != backupInfo[backupType].startPoint)
                || (backupInfo[backupType].lastBackupStatus.stopPoint != backupInfo[backupType].stopPoint))
        {
            /* Time slot has changed then start the backup again */
            backupInfo[backupType].isBackupStopdOnUserReq = FALSE;
        }
    }

    /* Do not start the backup if already stop by the user */
    if ((backupInfo[backupType].runFlg != STOP) || (TRUE == backupInfo[backupType].isBackupStopdOnUserReq))
    {
        MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
        return;
    }

    DPRINT(BACKUP_MANAGER, "backup time: [lastStart=%ld], [lastStop=%ld], [lastStatus=%d], [newStart=%ld], [newStop=%ld]",
           backupInfo[backupType].lastBackupStatus.startPoint, backupInfo[backupType].lastBackupStatus.stopPoint,
           backupInfo[backupType].lastBackupStatus.status, backupInfo[backupType].startPoint, backupInfo[backupType].stopPoint);
    backupInfo[backupType].runFlg = START;
    backupInfo[backupType].threadTerFlg = FALSE;
    MUTEX_UNLOCK(backupInfo[backupType].dataMutex);

    // create a thread which will starts the actual back up.
    if (FAIL == Utils_CreateThread(NULL, runScheduleBackup, &backupInfo[backupType], DETACHED_THREAD, RUN_SCHEDULEBKP_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(backupInfo[backupType].dataMutex);
        backupInfo[backupType].runFlg = STOP;
        backupInfo[backupType].threadTerFlg = TRUE;
        MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
        EPRINT(BACKUP_MANAGER, "fail to create schedule backup");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was callback of timer which is periodically run for every one minute. It will
 *          checks whether its time to start schedule back up as per configuration.
 * @param   count
 * @return  SUCCESS / FAIL
 */
static BOOL backupScheduler(UINT32 count)
{
    static UINT16				lastDayCnt = 0xffff;
	struct tm					brokenTime;
	SCHEDULE_BACKUP_CONFIG_t	schdlBackup;
	STORAGE_CONFIG_t 			storageConfig;

    /* Check and start schedule backup */
    checkScheduleBackup();

    if (SUCCESS != GetLocalTimeInBrokenTm(&brokenTime))
    {
        EPRINT(BACKUP_MANAGER, "fail to get local time to proc ScheduleBackup and RetentionMngmt");
        return FAIL;
    }

    /* Check record and backup retention */
    if (lastDayCnt == brokenTime.tm_yday)
    {
        return SUCCESS;
    }

    lastDayCnt = brokenTime.tm_yday;
    ReadStorageConfig(&storageConfig);
    if (storageConfig.recordRetentionStatus == ENABLE)
    {
        CleanupRecordFileByDay();
    }

    ReadScheduleBackupConfig(&schdlBackup);
    if ((storageConfig.backUpRetentionStatus == ENABLE) && (schdlBackup.scheduleBackup == ENABLE))
    {
        CleanupBkupFileByDay();
    }

	return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function writes event in to file.
 * @param   startTime
 * @param   endTime
 * @param   eventInfo
 * @param   backupType
 * @param   advaceDetails
 */
static void writeBackupEvent(time_t startTime, time_t endTime, UINT8 eventInfo, DM_BACKUP_TYPE_e backupType, CHARPTR advaceDetails)
{
    CHAR        detail[MAX_EVENT_DETAIL_SIZE];
    UINT8       detailLen;
    struct 	tm  brokenTime;

    if (SUCCESS != ConvertLocalTimeInBrokenTm(&startTime, &brokenTime))
	{
        EPRINT(BACKUP_MANAGER, "failed to get local time in broken time");
		return;
	}

    detailLen = snprintf(detail, sizeof(detail), "%02d/%02d/%02d-%02d~",
                         brokenTime.tm_mday, brokenTime.tm_mon + 1, (brokenTime.tm_year%100), brokenTime.tm_hour);
    ConvertLocalTimeInBrokenTm(&endTime, &brokenTime);
    if(detailLen > MAX_EVENT_DETAIL_SIZE)
    {
        EPRINT(BACKUP_MANAGER, "length is greater than buffer: [length=%d]", detailLen);
        detailLen = MAX_EVENT_DETAIL_SIZE;
    }

    snprintf(detail + detailLen, sizeof(detail) - detailLen, "%02d/%02d/%02d-%02d",
             brokenTime.tm_mday, brokenTime.tm_mon + 1, (brokenTime.tm_year%100), brokenTime.tm_hour);
    WriteEvent(LOG_SYSTEM_EVENT, (backupType == DM_MANUAL_BACKUP) ? LOG_MANUAL_BACKUP : LOG_SCHEDULE_BACKUP, detail, advaceDetails, eventInfo);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This thread function which will takes back from user selected time if record was present
 *          and also gives back in native format.
 * @param   threadArg
 * @return
 */
static VOIDPTR runManualBackup(VOIDPTR threadArg)
{
	BOOL					terminateFlg;
	BOOL					eventInfo = EVENT_COMPLETE;
	UINT32					startPoint;
	UINT32					stopPoint;
	DM_BACKUP_STATUS_e		backupStatus = BACKUP_RESP_MAX;
    DM_BACKUP_TYPE_e		backupType = DM_MANUAL_BACKUP;
	MANUAL_BACKUP_CONFIG_t	manualBkupCfg;
	UINT8					backUpFailReason = MAX_BACKUP_FAIL_REASON;
    BACKUP_INFO_t           *pBackupInfo = (BACKUP_INFO_t *)threadArg;

    THREAD_START("MANUAL_BACKUP");

    DPRINT(BACKUP_MANAGER, "manual backup started");
    MUTEX_LOCK(pBackupInfo->dataMutex);
    startPoint = pBackupInfo->startPoint;
    stopPoint = pBackupInfo->stopPoint;
    pBackupInfo->lastBackupStatus.startPoint = startPoint;
    pBackupInfo->lastBackupStatus.stopPoint = stopPoint;
    pBackupInfo->lastBackupStatus.status = BACKUP_HLT_START;
    pBackupInfo->backupStatus = BACKUP_IN_PROGRESS;
    setBackupStatusFile(backupType);
    MUTEX_UNLOCK(pBackupInfo->dataMutex);

    /* Generate Log of manual backup is started */
    writeBackupEvent(startPoint, stopPoint, EVENT_START, backupType, NULL);
    SetBackupLedState(BACKUP_GOING_ON);

    ReadManualBackupConfig(&manualBkupCfg);

    MUTEX_LOCK(pBackupInfo->dataMutex);
    terminateFlg = pBackupInfo->threadTerFlg;
    startPoint = pBackupInfo->lastBackupStatus.startPoint;
    stopPoint = pBackupInfo->lastBackupStatus.stopPoint;
    MUTEX_UNLOCK(pBackupInfo->dataMutex);

    while((startPoint < stopPoint) && (terminateFlg == FALSE))
    {
        switch(manualBkupCfg.backupLocation)
        {
            case MB_TO_USB_DEVICE:
                backupStatus = BackupToMedia(manualBkupCfg.camera, startPoint, backupType, BACKUP_ON_USB, abortBackup, DM_MANUAL_BACKUP);
                break;

            case MB_TO_NETWORK_DRIVE_1:
                backupStatus = BackupToMedia(manualBkupCfg.camera, startPoint, backupType, BACKUP_ON_NAS1, abortBackup, DM_MANUAL_BACKUP);
                break;

            case MB_TO_NETWORK_DRIVE_2:
                backupStatus = BackupToMedia(manualBkupCfg.camera, startPoint, backupType, BACKUP_ON_NAS2, abortBackup, DM_MANUAL_BACKUP);
                break;

            default:
                break;
        }

        if ((backupStatus == BACKUP_FAIL) || (backupStatus == BACKUP_NO_OPERATION_HDD))
        {
            EPRINT(BACKUP_MANAGER, "manual backup fail: [status=%d]", backupStatus);
            terminateFlg = TRUE;
            eventInfo = EVENT_INCOMPLETE;

            switch (backupStatus)
            {
                case BACKUP_NO_OPERATION_HDD:
                    backUpFailReason = BACKUP_FAIL_SOURCE_ERROR;
                    break;

                case BACKUP_FAIL:
                    backUpFailReason = BACKUP_FAIL_COPY_ERROR;
                    break;

                default:
                    backUpFailReason = BACKUP_FAIL_DISK_FAULT;
                    break;
            }
        }
        else
        {
            startPoint += SEC_IN_ONE_HOUR;
            MUTEX_LOCK(pBackupInfo->dataMutex);
            terminateFlg = pBackupInfo->threadTerFlg;
            MUTEX_UNLOCK(pBackupInfo->dataMutex);

            if(terminateFlg == TRUE)
            {
                eventInfo = EVENT_STOP;
                EPRINT(BACKUP_MANAGER, "user has aborted manual backup");
            }
        }
    }

    /* Generate log of manual backup complete or incomplete */
    writeBackupEvent(pBackupInfo->startPoint, pBackupInfo->stopPoint, eventInfo, backupType,
                     (backUpFailReason == MAX_BACKUP_FAIL_REASON) ? NULL : backupFailStr[backUpFailReason]);

    MUTEX_LOCK(pBackupInfo->dataMutex);
    pBackupInfo->lastBackupStatus.status = BACKUP_HLT_COMPLETE;
    pBackupInfo->backupStatus = (eventInfo == EVENT_INCOMPLETE) ? BACKUP_INCOMPLETE : BACKUP_COMPLETE;
    setBackupStatusFile(backupType);
    pBackupInfo->runFlg = STOP;
    MUTEX_UNLOCK(pBackupInfo->dataMutex);

    DPRINT(BACKUP_MANAGER, "manual backup is completed");
    SetBackupLedState(BACKUP_NO_ACITIVITY);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   If this function returns FALSE then no need to abort copy data. If this function returns
 *          TRUE then stop coping data.
 * @param   userData
 * @return  TRUE/FALSE
 */
static BOOL abortBackup(UINT32 userData)
{
    MUTEX_LOCK(backupInfo[userData].dataMutex);
    BOOL retVal = backupInfo[userData].threadTerFlg;
    MUTEX_UNLOCK(backupInfo[userData].dataMutex);

	if(retVal == TRUE)
	{
        DPRINT(BACKUP_MANAGER, "abort backup: [type=%s], [state=%d]", backupNameStr[userData], retVal);
	}
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This thread function which will takes back from user selected time if record was present
 *          and also gives back in native format.
 * @param   threadArg
 * @return
 */
static VOIDPTR runScheduleBackup(VOIDPTR threadArg)
{
    static BOOL					bootInitDoneF = FALSE;
	BOOL						terminateFlg;
	time_t						startPoint = 0;
	time_t						stopPoint = 0;
	struct timespec 			ts;
	DM_BACKUP_STATUS_e			backupStatus;
    DM_BACKUP_TYPE_e			backupType = DM_SCHEDULE_BACKUP;
	LOG_EVENT_STATE_e			eventState;
	SCHEDULE_BACKUP_CONFIG_t 	schdlBackup;
	BACKUP_DISK_STATUS_e		backupState;
	STORAGE_HEALTH_STATUS_e		healthStatus;
    BOOL 						status = SUCCESS;
    HDD_CONFIG_t                hddCfg = {0};
    BACKUP_INFO_t               *pBackupInfo = (BACKUP_INFO_t *)threadArg;

    THREAD_START("SCHEDULE_BACKUP");

    /* Wait for first call after bootup */
    if (bootInitDoneF == FALSE)
    {
        sleep(30);
        bootInitDoneF = TRUE;
    }

	//Read the schedule back up configuration
	ReadScheduleBackupConfig(&schdlBackup);

	// Check Destination Availble
    if (schdlBackup.backupLocation == SB_TO_USB_DEVICE)
	{
        USB_DISK_INFO_t backupDiskInfo;

        GetBackupDiskPara(backupType, &backupDiskInfo);
        if (backupDiskInfo.status == BACKUP_NO_DISK)
        {
            status = FAIL;
            MUTEX_LOCK(pBackupInfo->dataMutex);
            pBackupInfo->lastBackupStatus.status = BACKUP_HLT_INCOMPLETE;
            MUTEX_UNLOCK(pBackupInfo->dataMutex);
        }
	}
    else
	{
        if((SB_TO_NETWORK_DRIVE_1 == schdlBackup.backupLocation) || (SB_TO_NETWORK_DRIVE_2 == schdlBackup.backupLocation))
        {
            /* If the destination drive recovery is going on then take backup in next scheduling  */
            if(DISK_ACT_RECOVERY == GetStorageDriveStatus(schdlBackup.backupLocation - SB_TO_NETWORK_DRIVE_1))
            {
                DPRINT(BACKUP_MANAGER, "disk is in recovery, so rescheduling backup: [ndd=%d], [backupType=%d]",
                       (schdlBackup.backupLocation - SB_TO_NETWORK_DRIVE_1), backupType);
                MUTEX_LOCK(pBackupInfo->dataMutex);
                pBackupInfo->runFlg = STOP;
                MUTEX_UNLOCK(pBackupInfo->dataMutex);
                pthread_exit(NULL);
            }
        }
		status = CheckDestinationStatus(backupType, schdlBackup.backupLocation);
	}

    if (status == FAIL)
	{
		if(backUpFailReasonStatus != BACKUP_FAIL_NO_DISK)
		{
			backUpFailReasonStatus = BACKUP_FAIL_NO_DISK;
            EPRINT(BACKUP_MANAGER, "schedule backup disk not present");

            // Generate Log of schedule backup was fail
            writeBackupEvent(pBackupInfo->startPoint, pBackupInfo->stopPoint, EVENT_INCOMPLETE, backupType, backupFailStr[BACKUP_FAIL_NO_DISK]);

            // Set LED Indicator
            SetBackupLedState(NO_BACKUP_DEVICE_FOUND);
			TakeScheduleBackupFailSystemAction(INACTIVE);
			TakeScheduleBackupFailSystemAction(ACTIVE);
		}

        EPRINT(BACKUP_MANAGER, "schedule backup stops: [location=%d], [cause=%s]", schdlBackup.backupLocation, backupFailStr[BACKUP_FAIL_NO_DISK]);
        MUTEX_LOCK(pBackupInfo->dataMutex);
        pBackupInfo->runFlg = STOP;
        MUTEX_UNLOCK(pBackupInfo->dataMutex);
        pthread_exit(NULL);
	}

	// check Source availble
    ReadHddConfig(&hddCfg);
    if(LOCAL_HARD_DISK != hddCfg.recordDisk)
    {
        healthStatus = GetDiskHealthFunc(MAX_RECORDING_MODE);
        if(healthStatus != STRG_HLT_DISK_NORMAL)
        {
            if(((STRG_HLT_DISK_FULL == healthStatus) || (STRG_HLT_LOW_MEMORY == healthStatus)) && (BACKUP_PREVIOUS_HOUR == prevHealthStatus))
            {
                prevHealthStatus = BACKUP_CURRENT_HOUR;
            }
            else if(((STRG_HLT_DISK_FULL == healthStatus) || (STRG_HLT_LOW_MEMORY == healthStatus)) && (BACKUP_CURRENT_HOUR == prevHealthStatus))
            {
                prevHealthStatus = BACKUP_STOP_HOUR;
            }
            else
            {
                status = FAIL;
                EPRINT(BACKUP_MANAGER, "schedule backup stops: [healthStatus=%d], [cause=%s]", healthStatus, backupFailStr[BACKUP_FAIL_SOURCE_ERROR]);
            }
        }
        else
        {
            prevHealthStatus = BACKUP_PREVIOUS_HOUR;
        }
    }

	if(status == FAIL)
	{
		if(backUpFailReasonStatus != BACKUP_FAIL_SOURCE_ERROR)
		{
			backUpFailReasonStatus = BACKUP_FAIL_SOURCE_ERROR;

            // Generate Log of schedule backup was fail
            writeBackupEvent(pBackupInfo->startPoint, pBackupInfo->stopPoint, EVENT_INCOMPLETE, backupType, backupFailStr[BACKUP_FAIL_SOURCE_ERROR]);
			TakeScheduleBackupFailSystemAction(INACTIVE);
			TakeScheduleBackupFailSystemAction(ACTIVE);
		}

        EPRINT(BACKUP_MANAGER, "schedule backup stops: [cause=%s]", backupFailStr[BACKUP_FAIL_SOURCE_ERROR]);
        MUTEX_LOCK(pBackupInfo->dataMutex);
        pBackupInfo->runFlg = STOP;
        MUTEX_UNLOCK(pBackupInfo->dataMutex);
        pthread_exit(NULL);
	}

    // Generate Log of schedule backup was started
    MUTEX_LOCK(pBackupInfo->dataMutex);
    writeBackupEvent(pBackupInfo->startPoint, pBackupInfo->stopPoint, EVENT_START, backupType, NULL);
    pBackupInfo->lastBackupStatus.startPoint = pBackupInfo->startPoint;
    pBackupInfo->lastBackupStatus.stopPoint = pBackupInfo->stopPoint;
    pBackupInfo->lastBackupStatus.status = BACKUP_HLT_START;
	setBackupStatusFile(backupType);
    terminateFlg = pBackupInfo->threadTerFlg;
    MUTEX_UNLOCK(pBackupInfo->dataMutex);

    backupState = (schdlBackup.backupLocation == SB_TO_USB_DEVICE) ? BACKUP_IN_PROGRESS : BACKUP_DISABLE;
    DPRINT(BACKUP_MANAGER, "schedule backup in progress: [location=%d]", schdlBackup.backupLocation);

    MUTEX_LOCK(pBackupInfo->dataMutex);
    startPoint = pBackupInfo->lastBackupStatus.startPoint;
    stopPoint = pBackupInfo->lastBackupStatus.stopPoint;
    if(schdlBackup.backupLocation == SB_TO_USB_DEVICE)
    {
        pBackupInfo->backupStatus = backupState;
    }
    MUTEX_UNLOCK(pBackupInfo->dataMutex);

    while((startPoint < stopPoint) && (terminateFlg == FALSE))
    {
        switch(schdlBackup.backupLocation)
        {
            default:
                backupStatus = BACKUP_FAIL;
                break;

            case SB_TO_USB_DEVICE:
                backupStatus = BackupToMedia(schdlBackup.camera, startPoint, backupType, BACKUP_ON_USB, abortBackup, DM_SCHEDULE_BACKUP);
                break;

            case SB_TO_NETWORK_DRIVE_1:
                backupStatus = BackupToMedia(schdlBackup.camera, startPoint, backupType, BACKUP_ON_NAS1, abortBackup, DM_SCHEDULE_BACKUP);
                break;

            case SB_TO_NETWORK_DRIVE_2:
                backupStatus = BackupToMedia(schdlBackup.camera, startPoint, backupType, BACKUP_ON_NAS2, abortBackup, DM_SCHEDULE_BACKUP);
                break;

            case SB_TO_FTP_SERVER_1:
                backupStatus = BackupToFtp(schdlBackup.camera, startPoint, backupType, FTP_SERVER1, abortBackup, DM_SCHEDULE_BACKUP);
                break;

            case SB_TO_FTP_SERVER_2:
                backupStatus = BackupToFtp(schdlBackup.camera, startPoint, backupType, FTP_SERVER2, abortBackup, DM_SCHEDULE_BACKUP);
                break;
        }

        DPRINT(BACKUP_MANAGER, "schedule backup: [CurSts=%d], [LastSts=%d], [startPoint=%lu], [stopPoint=%lu]",
               backupStatus, pBackupInfo->lastBackupStatus.status, startPoint, stopPoint);

        switch(backupStatus)
        {
            default:
            {
                MUTEX_LOCK(pBackupInfo->dataMutex);
                pBackupInfo->threadTerFlg = TRUE;
                MUTEX_UNLOCK(pBackupInfo->dataMutex);
            }
            break;

            case BACKUP_SUCCESS:
            {
                startPoint += SEC_IN_ONE_HOUR;
                MUTEX_LOCK(pBackupInfo->dataMutex);
                if(pBackupInfo->lastBackupStatus.status == BACKUP_HLT_START)
                {
                    MUTEX_UNLOCK(pBackupInfo->dataMutex);
                    break;
                }
                pBackupInfo->lastBackupStatus.status = BACKUP_HLT_START;
                MUTEX_UNLOCK(pBackupInfo->dataMutex);

                writeBackupEvent(pBackupInfo->startPoint, pBackupInfo->stopPoint, EVENT_START, backupType, NULL);
                TakeScheduleBackupFailSystemAction(INACTIVE);
                DPRINT(BACKUP_MANAGER, "schedule backup starts");
            }
            break;

            case BACKUP_FAIL:
            {
                terminateFlg = TRUE;
                MUTEX_LOCK(pBackupInfo->dataMutex);
                pBackupInfo->threadTerFlg = TRUE;
                MUTEX_UNLOCK(pBackupInfo->dataMutex);

                if (backUpFailReasonStatus == BACKUP_FAIL_COPY_ERROR)
                {
                    break;
                }

                backUpFailReasonStatus = BACKUP_FAIL_COPY_ERROR;
                writeBackupEvent(pBackupInfo->startPoint, pBackupInfo->stopPoint,
                                 EVENT_INCOMPLETE, backupType, backupFailStr[BACKUP_FAIL_COPY_ERROR]);
                EPRINT(BACKUP_MANAGER, "schedule backup stops: [cause=%s]", backupFailStr[backUpFailReasonStatus]);
                TakeScheduleBackupFailSystemAction(INACTIVE);
                TakeScheduleBackupFailSystemAction(ACTIVE);
            }
            break;

            case BACKUP_DISK_FULL:
            {
                backupState = BACKUP_FULL;
            }
            /* FALL THROUGH */
            case BACKUP_NO_OPERATION_HDD:
            case BACKUP_FTP_CONN_FAIL:
            {
                MUTEX_LOCK(pBackupInfo->dataMutex);
                terminateFlg = pBackupInfo->threadTerFlg;
                EPRINT(BACKUP_MANAGER, "schedule backup stops: [terminateFlg=%d]", terminateFlg);

                if(pBackupInfo->lastBackupStatus.status != BACKUP_HLT_STOP)
                {
                    pBackupInfo->lastBackupStatus.status = BACKUP_HLT_STOP;
                    setBackupStatusFile(backupType);
                    MUTEX_UNLOCK(pBackupInfo->dataMutex);

                    if(backupStatus != BACKUP_DISK_FULL)
                    {
                        if(backupStatus == BACKUP_NO_OPERATION_HDD)
                        {
                            if(backUpFailReasonStatus != BACKUP_FAIL_SOURCE_ERROR)
                            {
                                backUpFailReasonStatus = BACKUP_FAIL_SOURCE_ERROR;
                                status = FALSE;
                            }
                        }
                        else
                        {
                            if(backUpFailReasonStatus != BACKUP_FAIL_DISK_FAULT)
                            {
                                backUpFailReasonStatus = BACKUP_FAIL_DISK_FAULT;
                                status = FALSE;
                            }
                        }
                        EPRINT(BACKUP_MANAGER, "schedule backup stops: [cause=%s]", backupFailStr[backUpFailReasonStatus]);
                    }
                    else
                    {
                        if(backUpFailReasonStatus != BACKUP_FAIL_DISK_FULL)
                        {
                            // Set LED Indicator
                            SetBackupLedState(BACKUP_DEVICE_NOT_ENOUGH_SPACE);
                            backUpFailReasonStatus = BACKUP_FAIL_DISK_FULL;
                            MUTEX_LOCK(pBackupInfo->dataMutex);
                            pBackupInfo->backupStatus = backupState;
                            MUTEX_UNLOCK(pBackupInfo->dataMutex);
                            status = FALSE;
                        }
                        EPRINT(BACKUP_MANAGER, "schedule backup stops: [cause=%s]", backupFailStr[backUpFailReasonStatus]);
                    }

                    if(status == FALSE)
                    {
                        writeBackupEvent(pBackupInfo->startPoint, pBackupInfo->stopPoint,
                                         EVENT_INCOMPLETE, backupType,backupFailStr[backUpFailReasonStatus]);
                        TakeScheduleBackupFailSystemAction(INACTIVE);
                        TakeScheduleBackupFailSystemAction(ACTIVE);
                    }
                    EPRINT(BACKUP_MANAGER, "schedule backup stops: [cause=%s]", backupFailStr[backUpFailReasonStatus]);
                }
                else
                {
                    MUTEX_UNLOCK(pBackupInfo->dataMutex);
                }

                if(terminateFlg == FALSE)
                {
                    clock_gettime(CLOCK_REALTIME, &ts);
                    ts.tv_sec += SEC_IN_ONE_MIN;
                    MUTEX_LOCK(pBackupInfo->condMutex);
                    pthread_cond_timedwait(&pBackupInfo->condSignal, &pBackupInfo->condMutex, &ts);
                    MUTEX_UNLOCK(pBackupInfo->condMutex);
                }
            }
            break;
        }

        MUTEX_LOCK(pBackupInfo->dataMutex);
        terminateFlg = pBackupInfo->threadTerFlg;

        if((pBackupInfo->startPoint == pBackupInfo->lastBackupStatus.startPoint)
            && (pBackupInfo->stopPoint == pBackupInfo->lastBackupStatus.stopPoint))
        {
            MUTEX_UNLOCK(pBackupInfo->dataMutex);
            continue;
        }

        EPRINT(BACKUP_MANAGER, "last schedule backup incomplete: [lastStart=%ld], [lastStop=%ld], [newStart=%ld], [newStop=%ld]",
               pBackupInfo->lastBackupStatus.startPoint, pBackupInfo->lastBackupStatus.stopPoint, pBackupInfo->startPoint, pBackupInfo->stopPoint);
        pBackupInfo->lastBackupStatus.startPoint = pBackupInfo->startPoint;
        pBackupInfo->lastBackupStatus.stopPoint = pBackupInfo->stopPoint;
        pBackupInfo->lastBackupStatus.status = BACKUP_HLT_START;
        setBackupStatusFile(backupType);
        startPoint = pBackupInfo->lastBackupStatus.startPoint;
        stopPoint = pBackupInfo->lastBackupStatus.stopPoint;
        MUTEX_UNLOCK(pBackupInfo->dataMutex);
    }

    MUTEX_LOCK(pBackupInfo->dataMutex);
	if(terminateFlg == FALSE)
	{
        pBackupInfo->lastBackupStatus.status = BACKUP_HLT_COMPLETE;
		eventState = EVENT_COMPLETE;
		backupState = BACKUP_COMPLETE;
	}
	else
	{
        pBackupInfo->lastBackupStatus.status = BACKUP_HLT_INCOMPLETE;
		eventState = EVENT_INCOMPLETE;
		backupState = BACKUP_INCOMPLETE;
	}

	setBackupStatusFile(backupType);
    pBackupInfo->runFlg = STOP;

	if(schdlBackup.backupLocation == SB_TO_USB_DEVICE)
	{
        pBackupInfo->backupStatus = backupState;
	}
    MUTEX_UNLOCK(pBackupInfo->dataMutex);

    if((eventState == EVENT_INCOMPLETE) && (backupState == BACKUP_INCOMPLETE) &&
		(backUpFailReasonStatus != BACKUP_FAIL_DISK_FAULT) &&
		(backUpFailReasonStatus != BACKUP_FAIL_SOURCE_ERROR) &&
		(backUpFailReasonStatus != BACKUP_FAIL_NO_DISK) &&
        (backUpFailReasonStatus != BACKUP_FAIL_COPY_ERROR))
	{
		backUpFailReasonStatus = BACKUP_FAIL_DISK_FAULT;
        writeBackupEvent(pBackupInfo->startPoint, pBackupInfo->stopPoint, EVENT_INCOMPLETE, backupType, backupFailStr[backUpFailReasonStatus]);
		TakeScheduleBackupFailSystemAction(INACTIVE);
		TakeScheduleBackupFailSystemAction(ACTIVE);
        DPRINT(BACKUP_MANAGER, "schedule backup incomplete: [cause=%s]",backupFailStr[backUpFailReasonStatus]);
	}
	else if((eventState == EVENT_COMPLETE) && (backupState == BACKUP_COMPLETE))
	{
		backUpFailReasonStatus = MAX_BACKUP_FAIL_REASON;
        writeBackupEvent(pBackupInfo->startPoint, pBackupInfo->stopPoint, EVENT_COMPLETE, backupType, NULL);
		TakeScheduleBackupFailSystemAction(INACTIVE);
	}

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops all type backup because of source disk needs to format at that time.
 */
void StopBackupOnHddFormat(void)
{
	terminateAllBackup();
	prevHealthStatus = BACKUP_PREVIOUS_HOUR;
    DPRINT(BACKUP_MANAGER, "all backups are stopped");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get the last backup status
 * @param   temp
 * @param   backupType
 * @return  Status
 */
BOOL GetBackupStatus(UINT8 temp, UINT8 backupType)
{
    MUTEX_LOCK(backupInfo[backupType].dataMutex);
    BACKUP_HLT_STATUS_e status = backupInfo[backupType].lastBackupStatus.status;
    MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives actual status of backup.
 * @param   backupType
 * @return  Status
 */
BACKUP_DISK_STATUS_e GetBackupInfo(UINT8 backupType)
{
    MUTEX_LOCK(backupInfo[backupType].dataMutex);
    BACKUP_DISK_STATUS_e status = backupInfo[backupType].backupStatus;
    MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks if Manual back-up already running, If not then creates a thread to
 *          Take Back of requested records.
 * @param   records
 * @param   recStorageDrive
 * @param   noOfRecords
 * @param   format
 * @param   callback
 * @param   connFd
 * @param   userName
 * @param   backupLocation
 * @param   callbackType
 * @return  Status
 */
NET_CMD_STATUS_e ManualBackUpOfRecords(SEARCH_RESULT_t *records, RECORD_ON_DISK_e recStorageDrive, UINT8 noOfRecords,
                                       BACKUP_FORMAT_e format, NW_CMD_BKUP_RCD_CB callback, INT32 connFd, CHARPTR userName,
                                       MB_LOCATION_e backupLocation, CLIENT_CB_TYPE_e callbackType)
{
    DM_BACKUP_TYPE_e	backupType = DM_MANUAL_BACKUP;
    BKUP_RCD_PARAM_t	*bkupRecordParam;

    if (format != BACKUP_FORMAT_AVI)
	{
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    if (backupLocation >= MAX_MB_LOCATION)
    {
        MANUAL_BACKUP_CONFIG_t manualBackupCfg;
        ReadManualBackupConfig(&manualBackupCfg);
        backupLocation = manualBackupCfg.backupLocation;
    }

    if (backupLocation == MB_TO_USB_DEVICE)
    {
        USB_DISK_INFO_t backupDiskInfo;

        if (SUCCESS != GetBackupDiskPara(backupType, &backupDiskInfo))
        {
            EPRINT(BACKUP_MANAGER, "fail to get usb info for backup: [type=%s]", backupNameStr[backupType]);
            return CMD_PROCESS_ERROR;
        }

        if (backupDiskInfo.status == BACKUP_DISK_FORMATTING)
        {
            return CMD_FORMAT_IN_PROCESS;
        }

        if (backupDiskInfo.status != BACKUP_DISK_DETECTED)
        {
            return CMD_NO_DISK_FOUND;
        }
    }
    else
    {
        DISK_VOLUME_INFO_t nddVolInfo;

        if (SUCCESS != GetNddVolumeInfo((backupLocation - MB_TO_NETWORK_DRIVE_1),&nddVolInfo))
        {
            EPRINT(BACKUP_MANAGER, "fail to get ndd info for backup: [type=%s], [ndd=%d]",
                   backupNameStr[backupType], (backupLocation - MB_TO_NETWORK_DRIVE_1));
            return CMD_PROCESS_ERROR;
        }

        if (nddVolInfo.status == DM_DISK_VOL_FORMATTING)
        {
            return CMD_FORMAT_IN_PROCESS;
        }

        if (nddVolInfo.status != DM_DISK_VOL_NORMAL)
        {
            return CMD_NO_DISK_FOUND;
        }
    }

    MUTEX_LOCK(backupInfo[backupType].dataMutex);
    if(backupInfo[backupType].runFlg == START)
    {
        MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
        return CMD_BACKUP_IN_PROCESS;
    }
    backupInfo[backupType].runFlg = START;
    backupInfo[backupType].threadTerFlg = FALSE;
    MUTEX_UNLOCK(backupInfo[backupType].dataMutex);

    /* PARASOFT: Allocated memory release in thread handler */
    bkupRecordParam = (BKUP_RCD_PARAM_t *)malloc(sizeof(BKUP_RCD_PARAM_t));
    if (bkupRecordParam == NULL)
    {
        MUTEX_LOCK(backupInfo[backupType].dataMutex);
        backupInfo[backupType].runFlg = STOP;
        MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
        return CMD_PROCESS_ERROR;
    }

    memcpy(bkupRecordParam->records, records, (noOfRecords * sizeof(SEARCH_RESULT_t)));
    bkupRecordParam->noOfRecords = noOfRecords;
    bkupRecordParam->callback = callback;
    bkupRecordParam->clientCbType = callbackType;
    bkupRecordParam->connFd = connFd;
    bkupRecordParam->format = format;
    bkupRecordParam->recStorageDrive = recStorageDrive;
    snprintf(bkupRecordParam->userName, MAX_USER_ACCOUNT_USERNAME_WIDTH, "%s", userName);
    bkupRecordParam->backupLocation = backupLocation;

    /* Create Record Backup Thread */
    if (FAIL == Utils_CreateThread(NULL, backUpRecordThread, bkupRecordParam, DETACHED_THREAD, BKP_REC_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(backupInfo[backupType].dataMutex);
        backupInfo[backupType].runFlg = STOP;
        backupInfo[backupType].threadTerFlg = TRUE;
        MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
        EPRINT(BACKUP_MANAGER, "fail to create manual backup thread");
        free(bkupRecordParam);
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   stopBackup
 * @param   backupType
 * @return  Status
 */
static NET_CMD_STATUS_e stopBackup(DM_BACKUP_TYPE_e backupType)
{
    //Check manual backup was going on
    MUTEX_LOCK(backupInfo[backupType].dataMutex);
    if (backupInfo[backupType].runFlg == STOP)
	{
        MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
        return CMD_BACKUP_NOT_IN_PROG;
    }
    backupInfo[backupType].threadTerFlg = TRUE;
    MUTEX_UNLOCK(backupInfo[backupType].dataMutex);

    MUTEX_LOCK(backupInfo[backupType].condMutex);
    pthread_cond_signal(&backupInfo[backupType].condSignal);
    MUTEX_UNLOCK(backupInfo[backupType].condMutex);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks if Manual back-up already running, If not then creates a thread to
 *          Take Back of requested records.
 * @param   records
 * @param   recStorageDrive
 * @param   noOfRecords
 * @param   callback
 * @param   connFd
 * @param   userName
 * @param   clientCbType
 * @return
 */
NET_CMD_STATUS_e ClipBackUpOfRecords(SYNC_CLIP_DATA_t *records, RECORD_ON_DISK_e recStorageDrive, UINT8 noOfRecords,
                                     NW_CMD_BKUP_RCD_CB callback, INT32 connFd, CHARPTR userName, CLIENT_CB_TYPE_e clientCbType)
{
	DM_BACKUP_TYPE_e		backupType = DM_MANUAL_BACKUP;
	USB_DISK_INFO_t			backupDiskInfo;
    BKUP_SYNC_CLIP_PARAM_t	*bkupRecordParam;
	MANUAL_BACKUP_CONFIG_t  manualCnfg;
	DISK_VOLUME_INFO_t	    nddVolInfo;

	ReadManualBackupConfig(&manualCnfg);

    /* if manual backup is set to USB Drive */
	if(manualCnfg.backupLocation == MB_TO_USB_DEVICE)
	{
        if (SUCCESS != GetBackupDiskPara(backupType, &backupDiskInfo))
        {
            EPRINT(BACKUP_MANAGER, "fail to get usb info for backup: [type=%s]", backupNameStr[backupType]);
            return CMD_PROCESS_ERROR;
        }
		nddVolInfo.status = DM_DISK_VOL_MAX;
	}
    else /* if manual backup is set to NDD */
	{
        if (SUCCESS != GetNddVolumeInfo((manualCnfg.backupLocation - MB_TO_NETWORK_DRIVE_1), &nddVolInfo))
        {
            EPRINT(BACKUP_MANAGER, "fail to get ndd info for backup: [type=%s], [ndd=%d]",
                   backupNameStr[backupType], (manualCnfg.backupLocation - MB_TO_NETWORK_DRIVE_1));
            return CMD_PROCESS_ERROR;
        }
		backupDiskInfo.status = BACKUP_DISK_UNPLUGGING;
	}

    /* go ahead if backup drive status is normal */
	if((backupDiskInfo.status == BACKUP_DISK_DETECTED) || (nddVolInfo.status == DM_DISK_VOL_NORMAL))
	{
        MUTEX_LOCK(backupInfo[backupType].dataMutex);

        /* if backup is stop then start else send backup in progress */
        if(backupInfo[backupType].runFlg == START)
		{
            MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
            return CMD_BACKUP_IN_PROCESS;
        }
        backupInfo[backupType].runFlg = START;
        backupInfo[backupType].threadTerFlg = FALSE;
        MUTEX_UNLOCK(backupInfo[backupType].dataMutex);

        bkupRecordParam	= (BKUP_SYNC_CLIP_PARAM_t *)malloc(sizeof(BKUP_SYNC_CLIP_PARAM_t));
        if (bkupRecordParam == NULL)
        {
            MUTEX_LOCK(backupInfo[backupType].dataMutex);
            backupInfo[backupType].runFlg = STOP;
            MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
            return CMD_PROCESS_ERROR;
        }

        memcpy(bkupRecordParam->records, records, (noOfRecords * sizeof(SYNC_CLIP_DATA_t)));
        bkupRecordParam->noOfRecords = noOfRecords;
        bkupRecordParam->callback = callback;
        bkupRecordParam->clientCbType = clientCbType;
        bkupRecordParam->connFd = connFd;
        bkupRecordParam->recStorageDrive = recStorageDrive;
        snprintf(bkupRecordParam->userName, MAX_USER_ACCOUNT_USERNAME_WIDTH, "%s", userName);

        // create a thread which will starts the actual back up
        if (FAIL == Utils_CreateThread(NULL, backUpClipRecordThread, bkupRecordParam, DETACHED_THREAD, CLIP_BKP_REC_THREAD_STACK_SZ))
        {
            MUTEX_LOCK(backupInfo[backupType].dataMutex);
            backupInfo[backupType].runFlg = STOP;
            backupInfo[backupType].threadTerFlg = TRUE;
            MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
            EPRINT(BACKUP_MANAGER, "fail to create backup clip record thread");
            free(bkupRecordParam);
            return CMD_PROCESS_ERROR;
        }

        DPRINT(BACKUP_MANAGER, "actual backup started: [noOfRecords=%d]", noOfRecords);
	}
    else if ((backupDiskInfo.status == BACKUP_DISK_FORMATTING) || (nddVolInfo.status == DM_DISK_VOL_FORMATTING))
	{
        return CMD_FORMAT_IN_PROCESS;
	}
	else
	{
        return CMD_NO_DISK_FOUND;
	}

    //Deallocate memory in thread function when CMD_SUCCESS response
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setBackupStatusFile
 * @param   backupType
 * @return  SUCCESS/FAIL
 */
static BOOL setBackupStatusFile(DM_BACKUP_TYPE_e backupType)
{
	CHAR 	fileName[64];
	INT32 	fileFd;

    snprintf(fileName, sizeof(fileName), BACKUP_STATUS_FILE"%d", backupType);
    fileFd = open(fileName, CREATE_RDWR_SYNC_MODE, USR_RW_GRP_RW_OTH_RW);
	if(fileFd == INVALID_FILE_FD)
	{
        EPRINT(BACKUP_MANAGER, "fail to open backup status file: [path=%s], [err=%s]", fileName, STR_ERR);
        return FAIL;
	}

    if (write(fileFd, &backupInfo[backupType].lastBackupStatus, sizeof(BACKUP_STATUS_t)) < (INT32)sizeof(BACKUP_STATUS_t))
    {
        EPRINT(BACKUP_MANAGER, "fail to write backup status file: [path=%s], [err=%s]", fileName, STR_ERR);
        close(fileFd);
        return FAIL;
    }

    close(fileFd);
    DPRINT(BACKUP_MANAGER, "backup status file write successfully: [status=%s]", backupStatusStr[backupInfo[backupType].lastBackupStatus.status]);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getBackupStatusFile
 * @param   backupType
 * @return  SUCCESS/FAIL
 */
static BOOL getBackupStatusFile(DM_BACKUP_TYPE_e backupType)
{
    BOOL                        retVal = FAIL;
    CHAR                        fileName[64];
    INT32                       fileFd;
    SCHEDULE_BACKUP_CONFIG_t    schdlBckCfg;

    snprintf(fileName, sizeof(fileName), BACKUP_STATUS_FILE"%d", backupType);
    fileFd = open(fileName, CREATE_RDWR_SYNC_MODE, USR_RW_GRP_RW_OTH_RW);
	if(fileFd == INVALID_FILE_FD)
	{
        EPRINT(BACKUP_MANAGER, "fail to open backup status file: [path=%s], [err=%s]", fileName, STR_ERR);
	}
	else
	{
        if(read(fileFd, &backupInfo[backupType].lastBackupStatus, sizeof(BACKUP_STATUS_t)) < (INT32)sizeof(BACKUP_STATUS_t))
		{
            EPRINT(BACKUP_MANAGER, "fail to read backup status file: [path=%s], [err=%s]", fileName, STR_ERR);
			backupInfo[backupType].lastBackupStatus.startPoint = 0;
			backupInfo[backupType].lastBackupStatus.stopPoint = 0;
			backupInfo[backupType].lastBackupStatus.status = BACKUP_HLT_INCOMPLETE;
		}
		else
		{
            DPRINT(BACKUP_MANAGER, "backup status file read successfully: [status=%s]", backupStatusStr[backupInfo[backupType].lastBackupStatus.status]);
			retVal = SUCCESS;
		}

		close(fileFd);
	}

	ReadScheduleBackupConfig(&schdlBckCfg);
	if(schdlBckCfg.scheduleBackup == DISABLE)
	{
		backupInfo[backupType].lastBackupStatus.status = BACKUP_HLT_DISABLE;
	}
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   terminateAllBackup
 */
static void terminateAllBackup(void)
{
	BOOL				status;
	DM_BACKUP_TYPE_e	backupType;

	for(backupType = 0; backupType < DM_MAX_BACKUP; backupType++)
	{
		stopBackup(backupType);
	}

	for(backupType = 0; backupType < DM_MAX_BACKUP; backupType++)
	{
		do
		{
			sleep(1);
            MUTEX_LOCK(backupInfo[backupType].dataMutex);
			status = backupInfo[backupType].runFlg;
            MUTEX_UNLOCK(backupInfo[backupType].dataMutex);

        } while(status == START);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   TerminateBackup
 * @param   nasDriveNo
 */
void TerminateBackup(UINT8 nasDriveNo)
{
	BOOL						status;
	SCHEDULE_BACKUP_CONFIG_t	schdlBackup;

	ReadScheduleBackupConfig(&schdlBackup);
    if(schdlBackup.backupLocation != (UINT8)(nasDriveNo + SB_TO_NETWORK_DRIVE_1))
	{
        return;
    }

    stopBackup(DM_SCHEDULE_BACKUP);

    do
    {
        sleep(1);
        MUTEX_LOCK(backupInfo[DM_SCHEDULE_BACKUP].dataMutex);
        status = backupInfo[DM_SCHEDULE_BACKUP].runFlg;
        MUTEX_UNLOCK(backupInfo[DM_SCHEDULE_BACKUP].dataMutex);

    } while(status == START);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback function for writting event related to back-up
 * @param   privData
 * @param   dataSize
 * @param   checkStopFlag
 * @return
 */
static BOOL backUpRecordEventCb(VOIDPTR privData, UINT64 dataSize, BOOL checkStopFlag)
{
	BKUP_RCD_PARAM_t	*bkupRecordParam = (BKUP_RCD_PARAM_t *)privData;
	CHAR				eventDetail[4];
	UINT8				percent;
	BOOL				retVal = TRUE;

	if(checkStopFlag == TRUE)
	{
        MUTEX_LOCK(backupInfo[DM_MANUAL_BACKUP].dataMutex);
		if (backupInfo[DM_MANUAL_BACKUP].threadTerFlg == TRUE)
		{
            EPRINT(BACKUP_MANAGER, "user has aborted manual avi backup");
			retVal = FALSE;
		}
        MUTEX_UNLOCK(backupInfo[DM_MANUAL_BACKUP].dataMutex);
        return retVal;
	}

    percent = (UINT8)(((float)(bkupRecordParam->processedSize + dataSize) / (float)bkupRecordParam->totalSize) * 100);

    // hold user At BACKUP_END_PERCENT. so that Avi header get written into file.
    if (percent <= BACKUP_END_PERCENT)
    {
        snprintf(eventDetail, sizeof(eventDetail), "%3d", percent);
        WriteEvent(LOG_OTHER_EVENT, LOG_MANUAL_BACKUP_STATUS, eventDetail, NULL, EVENT_ACTIVE);
    }
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback function for writting event related to back-up
 * @param   privData
 * @param   dataSize
 * @param   checkStopFlag
 * @return
 */
static BOOL backUpSyncRecordEventCb(VOIDPTR privData, UINT64 dataSize, BOOL checkStopFlag)
{
	BKUP_SYNC_CLIP_PARAM_t	*bkupRecordParam = (BKUP_SYNC_CLIP_PARAM_t *)privData;
	CHAR					eventDetail[4];
	UINT8					percent;
	BOOL					retVal = TRUE;

    if(checkStopFlag == TRUE)
    {
        MUTEX_LOCK(backupInfo[DM_MANUAL_BACKUP].dataMutex);
		if (backupInfo[DM_MANUAL_BACKUP].threadTerFlg == TRUE)
		{
            EPRINT(BACKUP_MANAGER, "user has aborted manual avi backup");
			retVal = FALSE;
		}
        MUTEX_UNLOCK(backupInfo[DM_MANUAL_BACKUP].dataMutex);
        return retVal;
	}

    percent = (UINT8)(((float)(bkupRecordParam->processedSize + dataSize) / (float)bkupRecordParam->totalSize) * 100);
    // hold user At BACKUP_END_PERCENT so that Avi header get written into file.
    if (percent <= BACKUP_END_PERCENT)
    {
        snprintf(eventDetail, sizeof(eventDetail), "%3d", percent);
        WriteEvent(LOG_OTHER_EVENT, LOG_MANUAL_BACKUP_STATUS, eventDetail, NULL, EVENT_ACTIVE);
    }
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This thread take back-up in of requested records in requested form. Before taking back-up
 *          checks for sufficient space available in USB. If not than it replies respective in callback.
 * @param   threadArg
 * @return
 */
static VOIDPTR backUpRecordThread(VOIDPTR threadArg)
{
	UINT8				loop;
	NET_CMD_STATUS_e	retVal = CMD_SUCCESS;
	LOG_EVENT_STATE_e	eventState;
	DM_BACKUP_TYPE_e 	backupType = DM_MANUAL_BACKUP;
    float				freeSize = 0;
    float 				tmpTotalSize = 0;
	USB_DISK_INFO_t		backupDiskInfo;
    BKUP_RCD_PARAM_t	*bkupRecordParam = (BKUP_RCD_PARAM_t *)threadArg;
    DISK_VOLUME_INFO_t 	nddVolInfo;
    MB_LOCATION_e       backupLocation = bkupRecordParam->backupLocation;;

    THREAD_START_INDEX("AVI_BACKUP", bkupRecordParam->noOfRecords);

    /* Backup thread priority is kept less than other thread as this job can be delayed */
	setpriority(PRIO_PROCESS, PRIO_PROCESS, 10);

    bkupRecordParam->totalSize = 0;
	for(loop = 0; loop < bkupRecordParam->noOfRecords; loop++)
	{
        if (GetSizeOfRecord(&bkupRecordParam->records[loop], bkupRecordParam->recStorageDrive, 1, &bkupRecordParam->recordSize[loop]) == FAIL)
		{
			retVal = CMD_PROCESS_ERROR;
			break;
		}

		bkupRecordParam->totalSize += bkupRecordParam->recordSize[loop];
	}

	// if any error has not occurred
	if (retVal == CMD_SUCCESS)
	{
        if(backupLocation == MB_TO_USB_DEVICE)
		{
			// Get USB Disk Parameters
			GetBackupDiskPara(backupType, &backupDiskInfo);
			if(backupDiskInfo.status == BACKUP_NO_DISK)
			{
				retVal = CMD_NO_DISK_FOUND;
			}
			else if(backupDiskInfo.status == BACKUP_DISK_FORMATTING)
			{
				retVal = CMD_FORMAT_IN_PROCESS;
			}
			else
			{
				sscanf(backupDiskInfo.freeSize, "%f", &freeSize);
			}
		}
		else
		{
            if(SUCCESS == GetNddVolumeInfo((backupLocation - MB_TO_NETWORK_DRIVE_1), &nddVolInfo))
			{
				if(nddVolInfo.status == DM_DISK_NOT_CONNECTED)
				{
					retVal = CMD_NO_DISK_FOUND;
				}
				else if(nddVolInfo.status == DM_DISK_VOL_FORMATTING)
				{
					retVal = CMD_FORMAT_IN_PROCESS;
				}
				else
				{
					sscanf(nddVolInfo.freeSize, "%f", &freeSize);
				}
			}
			else
			{
                EPRINT(BACKUP_MANAGER, "fail to get ndd info for backup: [type=%s], [ndd=%d]",
                       backupNameStr[backupType], (backupLocation - MB_TO_NETWORK_DRIVE_1));
			}
		}

		if (retVal == CMD_SUCCESS)
		{
			// NOTE:  Here 512 Bytes per record is considered extra for avoiding issues in writing AVI file
            tmpTotalSize = ((float)((float)(bkupRecordParam->totalSize + (512 * bkupRecordParam->noOfRecords))/GIGA_BYTE));
            if (freeSize < tmpTotalSize)
			{
                // Set LED Indicator
                SetBackupLedState(BACKUP_DEVICE_NOT_ENOUGH_SPACE);
                retVal = CMD_INVALID_FILE_SIZE;
			}
            DPRINT(BACKUP_MANAGER, "backup info: [recordSize=%f GB], [freeSize=%f GB]", tmpTotalSize, freeSize);
		}
	}

	if (bkupRecordParam->callback != NULL)
	{
        bkupRecordParam->callback(retVal, bkupRecordParam->connFd, freeSize, tmpTotalSize, TRUE, bkupRecordParam->clientCbType);
	}

	if (retVal == CMD_SUCCESS)
    {
        CHAR            details[MAX_EVENT_DETAIL_SIZE];
        CHAR            advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];
        SEARCH_RESULT_t *pStartRecord = &bkupRecordParam->records[0];
        SEARCH_RESULT_t *pEndRecord = &bkupRecordParam->records[bkupRecordParam->noOfRecords-1];

		// Set LED Indicator
        SetBackupLedState(BACKUP_GOING_ON);
        bkupRecordParam->processedSize = 0;

        snprintf(details, sizeof(details), "%02d/%02d/%02d-%02d:%02d~%02d/%02d/%02d-%02d:%02d",
                 pStartRecord->startTime.tm_mday, pStartRecord->startTime.tm_mon+1, pStartRecord->startTime.tm_year%100,
                 pStartRecord->startTime.tm_hour, pStartRecord->startTime.tm_min,
                 pEndRecord->endTime.tm_mday, pEndRecord->endTime.tm_mon+1, pEndRecord->endTime.tm_year%100,
                 pEndRecord->endTime.tm_hour, pEndRecord->endTime.tm_min);

        snprintf(advncDetail, sizeof(advncDetail), "%s | %s%d --> %s | %d",
                 bkupRecordParam->userName, pStartRecord->recStorageType ? NDD_DRIVE : SINGLE_DISK,
                 (pStartRecord->recStorageType == LOCAL_HARD_DISK) ? pStartRecord->diskId : pStartRecord->recStorageType,
                 (backupLocation == MB_TO_USB_DEVICE) ? "USB" : ((backupLocation == MB_TO_NETWORK_DRIVE_1) ? NDD_DRIVE"1" : NDD_DRIVE"2"),
                 bkupRecordParam->noOfRecords);
        WriteEvent(LOG_SYSTEM_EVENT, LOG_MANUAL_BACKUP, details, advncDetail, EVENT_START);

		for (loop = 0; loop < bkupRecordParam->noOfRecords; loop++)
		{
            DPRINT(BACKUP_MANAGER, "manual avi backup started: [record=%d], [size=%lld]", loop, bkupRecordParam->recordSize[loop]);

            if (BackUpRecordToManualDrive(&bkupRecordParam->records[loop], bkupRecordParam->recStorageDrive,
                                          backUpRecordEventCb, bkupRecordParam, (UINT64)(bkupRecordParam->totalSize / 100),
                                          backupLocation) != BACKUP_SUCCESS)
			{
                break;
			}

            // Check for thread termination flag
            bkupRecordParam->processedSize += bkupRecordParam->recordSize[loop];
            MUTEX_LOCK(backupInfo[backupType].dataMutex);
            if (backupInfo[backupType].threadTerFlg == TRUE)
            {
                MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
                EPRINT(BACKUP_MANAGER, "user has aborted manual avi backup");
                break;
            }
            MUTEX_UNLOCK(backupInfo[backupType].dataMutex);

            /* Get the last record */
            pEndRecord = &bkupRecordParam->records[loop];
		}

        /* If It's not the Last Record set event status to incomplete */
        if (loop < (bkupRecordParam->noOfRecords - 1))
        {
            eventState = EVENT_INCOMPLETE;
        }
        else
        {
            eventState = EVENT_COMPLETE;
            WriteEvent(LOG_OTHER_EVENT, LOG_MANUAL_BACKUP_STATUS, AVI_BACKUP_END_DETAIL, NULL, EVENT_ACTIVE);
            sleep(1);
        }

        snprintf(details, sizeof(details), "%02d/%02d/%02d-%02d:%02d~%02d/%02d/%02d-%02d:%02d",
                 pStartRecord->startTime.tm_mday, pStartRecord->startTime.tm_mon+1, pStartRecord->startTime.tm_year%100,
                 pStartRecord->startTime.tm_hour, pStartRecord->startTime.tm_min,
                 pEndRecord->endTime.tm_mday, pEndRecord->endTime.tm_mon+1, pEndRecord->endTime.tm_year%100,
                 pEndRecord->endTime.tm_hour, pEndRecord->endTime.tm_min);

        snprintf(advncDetail, sizeof(advncDetail), "%s | %s%d --> %s | %d",
                 bkupRecordParam->userName, pStartRecord->recStorageType ? NDD_DRIVE : SINGLE_DISK,
                 (pStartRecord->recStorageType == LOCAL_HARD_DISK) ? pStartRecord->diskId : pStartRecord->recStorageType,
                 (backupLocation == MB_TO_USB_DEVICE) ? "USB" : ((backupLocation == MB_TO_NETWORK_DRIVE_1) ? NDD_DRIVE"1" : NDD_DRIVE"2"),
                 (loop >= bkupRecordParam->noOfRecords) ? loop : loop+1);
        WriteEvent(LOG_SYSTEM_EVENT, LOG_MANUAL_BACKUP, details, advncDetail, eventState);

		// Set Led Indicator
		SetBackupLedState(BACKUP_NO_ACITIVITY);
	}

	// Free Allocated Memory for Search Records
	free(bkupRecordParam);
    MUTEX_LOCK(backupInfo[backupType].dataMutex);
	backupInfo[backupType].runFlg = STOP;
    MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This thread take back-up in of requested records in requested form. Before taking back-up
 *          checks for sufficient space available in USB. If not than it replies respective in callback.
 * @param   threadArg
 * @return
 */
static VOIDPTR backUpClipRecordThread(VOIDPTR threadArg)
{
    DM_BACKUP_STATUS_e      backupStatus;
    UINT8					loop;
	NET_CMD_STATUS_e		retVal = CMD_SUCCESS;
	LOG_EVENT_STATE_e		eventState;
	DM_BACKUP_TYPE_e		backupType = DM_MANUAL_BACKUP;
	float					freeSize = 0;
	float 					tmpTotalSize = 0;
	USB_DISK_INFO_t			backupDiskInfo;
    BKUP_SYNC_CLIP_PARAM_t	*bkupRecordParam = (BKUP_SYNC_CLIP_PARAM_t*)threadArg;
	MANUAL_BACKUP_CONFIG_t 	manualBackupCfg;
	DISK_VOLUME_INFO_t 	 	nddVolInfo;

	bkupRecordParam->totalSize = 0;

    THREAD_START_INDEX("CLIP_BACKUP", bkupRecordParam->noOfRecords);

    //backup thread priority is kept less than other thread as this job can be delayed
	setpriority(PRIO_PROCESS, PRIO_PROCESS, 10);

	for(loop = 0; loop < bkupRecordParam->noOfRecords; loop++)
	{
        if (GetSizeOfMultipleRecord(&bkupRecordParam->records[loop], bkupRecordParam->recStorageDrive, 1, &bkupRecordParam->recordSize[loop]) == FAIL)
		{
			retVal = CMD_PROCESS_ERROR;
			break;
		}
		bkupRecordParam->totalSize += bkupRecordParam->recordSize[loop];
	}

	// if any error has not occurred
	if (retVal == CMD_SUCCESS)
	{
		ReadManualBackupConfig(&manualBackupCfg);
		if(manualBackupCfg.backupLocation == MB_TO_USB_DEVICE)
		{
			// Get USB Disk Parameters
			GetBackupDiskPara(backupType, &backupDiskInfo);
			if(backupDiskInfo.status == BACKUP_NO_DISK)
			{
				retVal = CMD_NO_DISK_FOUND;
			}
			else if(backupDiskInfo.status == BACKUP_DISK_FORMATTING)
			{
				retVal = CMD_FORMAT_IN_PROCESS;
			}
			else
			{
				sscanf(backupDiskInfo.freeSize, "%f", &freeSize);
			}
		}
		else
		{
            if(SUCCESS == GetNddVolumeInfo((manualBackupCfg.backupLocation - MB_TO_NETWORK_DRIVE_1), &nddVolInfo))
			{
				if(nddVolInfo.status == DM_DISK_NOT_CONNECTED)
				{
					retVal = CMD_NO_DISK_FOUND;
				}
				else if(nddVolInfo.status == DM_DISK_VOL_FORMATTING)
				{
					retVal = CMD_FORMAT_IN_PROCESS;
				}
				else
				{
					sscanf(nddVolInfo.freeSize, "%f", &freeSize);
				}
			}
			else
			{
                EPRINT(BACKUP_MANAGER, "fail to get ndd info for backup: [type=%s], [ndd=%d]",
                       backupNameStr[backupType], (manualBackupCfg.backupLocation - MB_TO_NETWORK_DRIVE_1));
			}
		}

        if (retVal == CMD_SUCCESS)
        {
            /* NOTE: Here 512 Bytes per record is considered extra for avoiding issues in writing AVI file */
            tmpTotalSize = ((float)((float)(bkupRecordParam->totalSize + (512 * bkupRecordParam->noOfRecords))/GIGA_BYTE));
            if (freeSize < tmpTotalSize)
            {
                // Set LED Indicator
                SetBackupLedState(BACKUP_DEVICE_NOT_ENOUGH_SPACE);
                retVal = CMD_INVALID_FILE_SIZE;
            }
        }
        DPRINT(BACKUP_MANAGER, "backup info: [recordSize=%f GB], [freeSize=%f GB]", tmpTotalSize, freeSize);
	}

	if (bkupRecordParam->callback != NULL)
	{
        bkupRecordParam->callback(retVal, bkupRecordParam->connFd, freeSize, tmpTotalSize, TRUE, bkupRecordParam->clientCbType);
	}

	if (retVal == CMD_SUCCESS)
	{
		bkupRecordParam->processedSize = 0;
        MUTEX_LOCK(backupInfo[backupType].dataMutex);
		backupInfo[backupType].backupStatus = BACKUP_IN_PROGRESS;
        MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
		// Set LED Indicator
		SetBackupLedState(BACKUP_GOING_ON);
        WriteEvent(LOG_SYSTEM_EVENT, LOG_MANUAL_BACKUP, NULL, bkupRecordParam->userName, EVENT_START);
		eventState = EVENT_COMPLETE;

		for (loop = 0; loop < bkupRecordParam->noOfRecords; loop++)
		{
            DPRINT(BACKUP_MANAGER, "manual avi backup started: [record=%d], [size=%lld]", loop, bkupRecordParam->recordSize[loop]);

            backupStatus = BackUpSyncRecordToManualDrive(&bkupRecordParam->records[loop], bkupRecordParam->recStorageDrive,
                                                         backUpSyncRecordEventCb, bkupRecordParam, (UINT64)(bkupRecordParam->totalSize / 100));
            if (backupStatus != BACKUP_SUCCESS)
			{
                if (backupStatus == BACKUP_STOPPED)
                {
                    /* if backup is stopped by user */
                    eventState = EVENT_STOP;
                    EPRINT(BACKUP_MANAGER, "backup stopped by user");
                }
                else
                {
                    /* if backup is failed due to any other reason */
                    eventState = EVENT_INCOMPLETE;
                    EPRINT(BACKUP_MANAGER, "backup failed");
                }
                break;
			}

            // Check for thread termination flag
            bkupRecordParam->processedSize += bkupRecordParam->recordSize[loop];
            MUTEX_LOCK(backupInfo[backupType].dataMutex);
            if (backupInfo[backupType].threadTerFlg == FALSE)
            {
                MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
                continue;
            }
            MUTEX_UNLOCK(backupInfo[backupType].dataMutex);

            // If It's not the Last Record set event status to incomplete
            if (loop != (bkupRecordParam->noOfRecords - 1))
            {
                eventState = EVENT_INCOMPLETE;
            }
            EPRINT(BACKUP_MANAGER, "user has aborted manual avi backup");
            break;
        }

        if (eventState == EVENT_COMPLETE)
        {
            WriteEvent(LOG_OTHER_EVENT, LOG_MANUAL_BACKUP_STATUS, AVI_BACKUP_END_DETAIL, NULL, EVENT_ACTIVE);
            sleep(1);
        }

		// Set Led Indicator
		SetBackupLedState(BACKUP_NO_ACITIVITY);
        WriteEvent(LOG_SYSTEM_EVENT, LOG_MANUAL_BACKUP, NULL, bkupRecordParam->userName, eventState);

        MUTEX_LOCK(backupInfo[backupType].dataMutex);
		backupInfo[backupType].backupStatus = BACKUP_COMPLETE;
        MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
	}

	// Free Allocated Memory for Search Records
	free(bkupRecordParam);
    MUTEX_LOCK(backupInfo[backupType].dataMutex);
	backupInfo[backupType].runFlg = STOP;
    MUTEX_UNLOCK(backupInfo[backupType].dataMutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetBackupTermFlag
 * @return
 */
BOOL GetBackupTermFlag(void)
{
	return backupInfo[DM_MANUAL_BACKUP].threadTerFlg;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   SetBackupStopReq
 * @param   BackupType
 * @param   Status
 */
void SetBackupStopReq(DM_BACKUP_TYPE_e BackupType, BOOL Status)
{
    if (BackupType >= DM_MAX_BACKUP)
	{
        EPRINT(BACKUP_MANAGER, "invld back type: [type=%d]", BackupType);
        return;
    }

    MUTEX_LOCK(backupInfo[BackupType].dataMutex);
    backupInfo[BackupType].isBackupStopdOnUserReq = Status;
    MUTEX_UNLOCK(backupInfo[BackupType].dataMutex);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
