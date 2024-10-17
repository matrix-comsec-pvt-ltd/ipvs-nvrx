//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		NwDriveManager.h
@brief      This file contains all the functionality needed to use a network drive in NVR application.
            It mounts network drive with the requested system and does all other necessary settings
            for the application.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/mount.h>

/* Application Includes */
#include "NwDriveManager.h"
#include "NetworkController.h"
#include "Queue.h"
#include "RecordManager.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define NETWORK_DRIVE_MOUNT_PATH        MEDIA_DIR_PATH "/" NDD_DRIVE_NAME
#define NETWORK_TEST_DRIVE_MOUNT_PATH	MEDIA_DIR_PATH "/TEST_NDD%d/"

/* Example: mount -t cifs //192.168.101.102/Shared/NVR /matrix/media/NDD1 -o soft */
#if defined(RK3568_NVRL) || defined(RK3588_NVRH)
/* If echo_interval=X, It will send keep-alive at X seconds interval and will check connectivity at 2X for non-responsive cifs. There will be 3 retry */
#define MOUNT_CIFS_DRIVE                "mount -t cifs //%s/%s " MEDIA_DIR_PATH "/%sNDD%d -o soft,echo_interval=5"
#else
#define MOUNT_CIFS_DRIVE                "mount -t cifs //%s/%s " MEDIA_DIR_PATH "/%sNDD%d -o soft,cache=none"
#endif

/* Example: mount -t NFS -o nolock,sync,soft,timeo=30,retrans=1 192.168.101.102:Shared /matrix/media/NDD1 */
#define MOUNT_NFS_DRIVE                 "mount -t nfs -o nolock,soft,timeo=300,retrans=1,retry=0 %s:%s "

#if defined(OEM_JCI)
#define NDD_BASE_DIR_PREFIX             "NVR"
#else
#define NDD_BASE_DIR_PREFIX             "SATATYA"
#endif

#define MAKE_MATRIX_REC_DIR             "%s%s/" NDD_BASE_DIR_PREFIX "_RECORD/"
#define MAKE_MATRIX_BACKUP_DIR          "%s%s/" NDD_BASE_DIR_PREFIX "_BACKUP/"
#define MAKE_MATRIX_IMG_DIR             "%s%s/" NDD_BASE_DIR_PREFIX "_IMAGE/"
#define MAKE_MATRIX_TEST_DIR            "%s%s/" NDD_BASE_DIR_PREFIX "_TEST/"
#define TEST_FILE_NAME                  "Test_%02d%02d%04d%02d%02d%02d"
#define TEST_FILE_MESSAGE               "This is a test message sent by the Device."

#define NETWORK_DEVICE_TIMEOUT          (60)
#define NAS_STATUS_UPDATE_TIME          (30) /* Seconds */
#define MAX_NAS_REQUEST                 (MAX_CAMERA * 2)
#define NAS_UPLOAD_QUEUE_SIZE           (getMaxCameraForCurrentVariant() * 2)
#define	INVALID_NAS_HANDLE              (255)

/* Use Default Stack Size*/
#define MNT_STATUS_THREAD_STACK_SZ      (3 * MEGA_BYTE)
#define CONN_TEST_THREAD_STACK_SZ       (2 * MEGA_BYTE)
#define NAS_TRANSFER_THREAD_STACK_SZ    (2 * MEGA_BYTE)
#define UPDATE_CONFIG_THREAD_STACK_SZ   (3 * MEGA_BYTE)
#define CONN_NW_DRIVE_THREAD_STACK_SZ   (3 * MEGA_BYTE)
#define NAS_IMG_UPLOAD_THREAD_STACK_SZ  (3 * MEGA_BYTE)

#define STOP_NDD_FUNCTINALITY(nddIndex) {TerminateBackup(nddIndex);                         \
                                         RemovePlayBackForAllSessionId();                   \
                                         RemoveSyncPlayBackForAllSessionId(DISK_ACT_MAX);   \
                                         RemoveInstantPlayBackForAllSessionId();}

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
	NETWORK_DRIVE_STATUS_e	nwDriveStatus;
	STORAGE_HEALTH_STATUS_e	healthStatus;
	DISK_VOL_STATUS_e		volStatus;
	CHAR					nddMountPoint[MOUNT_POINT_SIZE];
	CHAR					mtxDirMountPoint[MAX_MATRIX_DIR][MOUNT_POINT_SIZE];
	DISK_SIZE_t				storageStatus;
	pthread_mutex_t			nwDriveInfoLock;

}NETWORK_DRIVE_INFO_t;

typedef struct
{
    NAS_CALLBACK            userCallback;
    UINT16                  userData;		// give back to user
    UINT8                   requestStatus;	// Indicates request in which state
    UINT8                   nasReqIdx;
    NAS_FILE_INFO_t         nasInfo;

}NAS_REQUEST_LIST_t;

typedef struct
{
    UINT8                   nddIdx;
    NETWORK_DRIVE_CONFIG_t  oldCopy;
    NETWORK_DRIVE_CONFIG_t  newCopy;
    pthread_mutex_t         nasIdxMutex;

}NDD_CFG_CHNG_t;

typedef struct
{
    UINT8                   nddIdx;
    QUEUE_HANDLE            nasQHandle;

}NDD_IMAGE_UPLD_t;

typedef struct
{
    BOOL                    runningState;
    UINT8                   nddIdx;
    NETWORK_DRIVE_CONFIG_t  nddConfig;
    NW_CMD_REPLY_CB         callBack;
    INT32                   clientSocket;
    pthread_mutex_t         testNddMutex;

}NDD_TEST_CONN_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void startNwDriveRetryTmr(UINT32 timeOut);
//-------------------------------------------------------------------------------------------------
static void retryNwDevice(UINT32 data);
//-------------------------------------------------------------------------------------------------
static VOIDPTR connectToNwDrive(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static void disconnectFromNwDrive(UINT8 nwDriveIdx);
//-------------------------------------------------------------------------------------------------
static BOOL makeNwDriveReadyToUse(UINT8 nwDriveIdx);
//-------------------------------------------------------------------------------------------------
static BOOL getNwDriveStorageStatus(UINT8 nwDriveIdx);
//-------------------------------------------------------------------------------------------------
static BOOL unmountNetworkDevice(CHARPTR mountPath);
//-------------------------------------------------------------------------------------------------
static BOOL formatNetworkDevice(CHARPTR	mntPoint);
//-------------------------------------------------------------------------------------------------
static VOIDPTR nasTransfer(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR nasImageUpload(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static void queueFullCb(VOIDPTR entry);
//-------------------------------------------------------------------------------------------------
static VOIDPTR updateNddConfig(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR connectTestNwDrive(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR checkNddMountStatus(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static void nddStatusUpdateTmrCb(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL isDriveReadyToUse(CHARPTR mntPoint, UINT32PTR pErrorCode);
//-------------------------------------------------------------------------------------------------
static BOOL getDomainAndUserName(CHARPTR pRcvdUserName, CHARPTR pDomain, CHARPTR pUserName);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static BOOL                     nddSizeUpdateNotifyF = FALSE;
static BOOL 					nddFuncStatus[MAX_NW_DRIVE];
static BOOL						nddUpdateStatus[MAX_NW_DRIVE];
static BOOL						comeBackNddUpdatedThread;
static BOOL 					nddVolBuildStatus;
static TIMER_HANDLE				nwDriveTmrHndl = INVALID_TIMER_HANDLE;
static TIMER_HANDLE				nwDriveUpdatedTmrHndl = INVALID_TIMER_HANDLE;
static pthread_mutex_t 			nddVolBuildMutex;
static pthread_mutex_t 			nasReqMutex;
static pthread_mutex_t 			nddFuncMutex;
static pthread_mutex_t			nddStatus;
static NAS_REQUEST_LIST_t		nasRequestList[MAX_NAS_REQUEST];
static NETWORK_DRIVE_INFO_t		nwDriveInfo[MAX_NW_DRIVE];
static NDD_CFG_CHNG_t			nddCfgChng;
static NDD_IMAGE_UPLD_t			nddImageUpld[MAX_NW_DRIVE];
static NDD_TEST_CONN_t			nddTestConn;
static pthread_cond_t 			waitForNddStatus;
static pthread_mutex_t 			condWaitForNddStatus;

static const CHARPTR nwMediaNameStr[MAX_NW_DRIVE] =
{
	"Network Drive 1",
	"Network Drive 2",
};

static const CHARPTR mtxDirStructure[MAX_MATRIX_DIR] =
{
	MAKE_MATRIX_REC_DIR,
	MAKE_MATRIX_BACKUP_DIR,
	MAKE_MATRIX_IMG_DIR,
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will initialize network drive module.
 */
void InitNwDriveManager(void)
{
	UINT8					nwDriveIdx;
	UINT8					nasRequestCnt = 0;
	NDD_MATRIX_DIR_e		dirCnt;
	NETWORK_DRIVE_CONFIG_t	nwConfig;
	QUEUE_INIT_t			qInfo;
    TIMER_INFO_t            timerInfo;

	nddVolBuildStatus = FALSE;
	comeBackNddUpdatedThread = TRUE;
    MUTEX_INIT(nddVolBuildMutex, NULL);
    MUTEX_INIT(nddFuncMutex, NULL);
    MUTEX_INIT(nddStatus, NULL);
    MUTEX_INIT(condWaitForNddStatus, NULL);
    MUTEX_LOCK(condWaitForNddStatus);
	pthread_cond_init(&waitForNddStatus, NULL);
    MUTEX_UNLOCK(condWaitForNddStatus);
	nddTestConn.runningState = NO;
    MUTEX_INIT(nddTestConn.testNddMutex, NULL);

    for(nwDriveIdx = 0; nwDriveIdx < MAX_NW_DRIVE; nwDriveIdx++)
	{
        MUTEX_INIT(nwDriveInfo[nwDriveIdx].nwDriveInfoLock, NULL);
		ReadSingleNetworkDriveConfig(nwDriveIdx, &nwConfig);
		nddFuncStatus[nwDriveIdx] = TRUE;
		nddUpdateStatus[nwDriveIdx] = TRUE;

        nwDriveInfo[nwDriveIdx].nwDriveStatus = (nwConfig.enable == TRUE) ? NETWORK_DRIVE_DISCONNECTED : NETWORK_DRIVE_DISABLE;
		nwDriveInfo[nwDriveIdx].storageStatus.freeSize  = 0;
		nwDriveInfo[nwDriveIdx].storageStatus.totalSize = 0;
		nwDriveInfo[nwDriveIdx].storageStatus.usedSize = 0;
		nwDriveInfo[nwDriveIdx].healthStatus = STRG_HLT_NO_DISK;
		nwDriveInfo[nwDriveIdx].volStatus = DM_DISK_VOL_MAX;
		nwDriveInfo[nwDriveIdx].nddMountPoint[0] = '\0';

        for(dirCnt = MATRIX_REC_DIR; dirCnt < MAX_MATRIX_DIR; dirCnt++)
		{
			nwDriveInfo[nwDriveIdx].mtxDirMountPoint[dirCnt][0] = '\0';
		}
	}

	nddCfgChng.nddIdx = MAX_NW_DRIVE;
    MUTEX_INIT(nddCfgChng.nasIdxMutex, NULL);
    startNwDriveRetryTmr((NETWORK_DEVICE_TIMEOUT/2));

    MUTEX_INIT(nasReqMutex, NULL);
	for(nasRequestCnt = 0; nasRequestCnt < MAX_NAS_REQUEST; nasRequestCnt++)
	{
		nasRequestList[nasRequestCnt].requestStatus = FREE;
        nasRequestList[nasRequestCnt].nasInfo.localFileName[0] = '\0';
        nasRequestList[nasRequestCnt].nasInfo.remoteFile[0] = '\0';
	}

	QUEUE_PARAM_INIT(qInfo);
	qInfo.sizoOfMember = sizeof(NAS_FILE_INFO_t);
	qInfo.maxNoOfMembers = NAS_UPLOAD_QUEUE_SIZE;
	qInfo.syncRW = TRUE;
	qInfo.overwriteOldest = TRUE;
	qInfo.callback = queueFullCb;

	for (nasRequestCnt = 0; nasRequestCnt < MAX_NAS_SERVER; nasRequestCnt++)
	{
		nddImageUpld[nasRequestCnt].nddIdx = nasRequestCnt;
		nddImageUpld[nasRequestCnt].nasQHandle = QueueCreate(&qInfo);
        if (FAIL == Utils_CreateThread(NULL, nasImageUpload, &nddImageUpld[nasRequestCnt], DETACHED_THREAD, NAS_IMG_UPLOAD_THREAD_STACK_SZ))
        {
            EPRINT(NDD_MANAGER, "fail to create nas img upld thread");
        }
	}

    /* Start NAS mount status and storage info timer */
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(NAS_STATUS_UPDATE_TIME);
    timerInfo.data = 0;
    timerInfo.funcPtr = nddStatusUpdateTmrCb;
    StartTimer(timerInfo, &nwDriveUpdatedTmrHndl);

    /* Check NAS status thread to check health periodically */
    if (FALSE == Utils_CreateThread(NULL, checkNddMountStatus, NULL, DETACHED_THREAD, MNT_STATUS_THREAD_STACK_SZ))
    {
        EPRINT(NDD_MANAGER, "fail to create ndd status thread");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will initialize network drive module.
 */
void DeInitNwDriveManager(void)
{
    UINT8 nwDeviceIdx;

	comeBackNddUpdatedThread = FALSE;
    MUTEX_LOCK(condWaitForNddStatus);
	pthread_cond_signal(&waitForNddStatus);
    MUTEX_UNLOCK(condWaitForNddStatus);

    for(nwDeviceIdx = 0; nwDeviceIdx < MAX_NW_DRIVE; nwDeviceIdx++)
	{
        disconnectFromNwDrive(nwDeviceIdx);
	}

	DeleteTimer(&nwDriveTmrHndl);
	DeleteTimer(&nwDriveUpdatedTmrHndl);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will start timer to connect with the configured network drive.
 * @param   timeOut
 */
static void	startNwDriveRetryTmr(UINT32 timeOut)
{
    if (nwDriveTmrHndl == INVALID_TIMER_HANDLE)
	{
        TIMER_INFO_t timerInfo;

		timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(timeOut);
        timerInfo.data = 0;
		timerInfo.funcPtr = retryNwDevice;
        StartTimer(timerInfo, &nwDriveTmrHndl);
        DPRINT(NDD_MANAGER, "retry timer is started");
	}
	else
	{
		ReloadTimer(nwDriveTmrHndl, timeOut);
        DPRINT(NDD_MANAGER, "retry timer is reloaded");
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will do all the necessary setups to use the network drive. It will mount
 *          the drive and finds its storage status for further use.
 * @param   data
 */
static void retryNwDevice(UINT32 data)
{
    DeleteTimer(&nwDriveTmrHndl);
    if (FALSE == Utils_CreateThread(NULL, connectToNwDrive, NULL, DETACHED_THREAD, CONN_NW_DRIVE_THREAD_STACK_SZ))
	{
        EPRINT(NDD_MANAGER, "fail to create initialisation thread");
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sets status of Network device volume is bulding or not.
 * @param   status
 */
void SetNddVolBuildStatus(BOOL status)
{
    MUTEX_LOCK(nddVolBuildMutex);
	nddVolBuildStatus = status;
    MUTEX_UNLOCK(nddVolBuildMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function get status of Network device volume is bulding or not
 * @return  status
 */
BOOL GetNddVolBuildStatus(void)
{
    MUTEX_LOCK(nddVolBuildMutex);
    BOOL status = nddVolBuildStatus;
    MUTEX_UNLOCK(nddVolBuildMutex);
	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sets status of Network device functionality.
 * @param   status
 * @param   diskId
 */
void SetNddNonFuncStatus(BOOL status, UINT8 diskId)
{
    MUTEX_LOCK(nddFuncMutex);
	nddFuncStatus[diskId] = status;
    MUTEX_UNLOCK(nddFuncMutex);
    DPRINT(NDD_MANAGER, "ndd function status: [ndd=%d], [status=%s]", diskId, status ? "non-usable" : "usable");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function get status of Network device Functionality.
 * @param   diskId
 * @return  status
 */
BOOL GetNddNonFuncStatus(UINT8 diskId)
{
    MUTEX_LOCK(nddFuncMutex);
    BOOL status = nddFuncStatus[diskId];
    MUTEX_UNLOCK(nddFuncMutex);
	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will do all the necessary setups to use the network drive. It will mount
 *          the drive and finds its storage status for further use.
 * @param   threadArg
 * @return
 */
static VOIDPTR connectToNwDrive(VOIDPTR threadArg)
{
    BOOL                    retryNwDriveF = FALSE;
    UINT8                   nwDriveIdx;
    NETWORK_DRIVE_CONFIG_t  nddConfig;
    HDD_CONFIG_t            hddConfig;
    RECORD_ON_DISK_e        recordDisk;

	ReadHddConfig(&hddConfig);
	SetNddVolBuildStatus(TRUE);

	for(nwDriveIdx = 0; nwDriveIdx < MAX_NW_DRIVE; nwDriveIdx++)
	{
        recordDisk = nwDriveIdx + REMOTE_NAS1;
        MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
        if (nwDriveInfo[nwDriveIdx].nwDriveStatus == NETWORK_DRIVE_MOUNTED)
        {
            MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
            SetStorageDriveStatus(recordDisk, DISK_ACT_NORMAL);
            continue;
        }

        if (nwDriveInfo[nwDriveIdx].nwDriveStatus != NETWORK_DRIVE_DISCONNECTED)
		{
            MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
            continue;
        }
        MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);

        if (hddConfig.recordDisk == recordDisk)
        {
            SetNddNonFuncStatus(TRUE, nwDriveIdx);
        }

        if (FAIL == makeNwDriveReadyToUse(nwDriveIdx))
        {
            ReadSingleNetworkDriveConfig(nwDriveIdx, &nddConfig);
            if (nddConfig.enable == ENABLE)
            {
                retryNwDriveF = TRUE;
            }

            /* If current drive is recording drive then only print the event */
            if (hddConfig.recordDisk == recordDisk)
            {
                WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(MAX_HDD_MODE, hddConfig.recordDisk), NULL, EVENT_NO_DISK);
            }
            continue;
        }

        if (getNwDriveStorageStatus(nwDriveIdx) == FAIL)
        {
            EPRINT(NDD_MANAGER, "storage information not received: [ndd=%s]", nwMediaNameStr[nwDriveIdx]);
        }

        /* If current drive is recording drive then only print the event */
        if (hddConfig.recordDisk == recordDisk)
        {
            SetNddNonFuncStatus(FALSE, nwDriveIdx);
            WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(MAX_HDD_MODE, hddConfig.recordDisk), NULL, EVENT_NORMAL);
        }

        SetStorageDriveStatus(recordDisk, DISK_ACT_NORMAL);
        UpdateRecordingMedia(recordDisk);
	}

	SetNddVolBuildStatus(FALSE);
    if (TRUE == retryNwDriveF)
	{
        startNwDriveRetryTmr(NETWORK_DEVICE_TIMEOUT);
	}

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will un-mount the requested network drive.
 * @param   nwDriveIdx
 */
static void disconnectFromNwDrive(UINT8 nwDriveIdx)
{
	NDD_MATRIX_DIR_e			dirCnt;
	SCHEDULE_BACKUP_CONFIG_t	scheduleBkupCfg;

    if(nwDriveIdx >= MAX_NW_DRIVE)
	{
        return;
    }

    MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
    if((nwDriveInfo[nwDriveIdx].nwDriveStatus != NETWORK_DRIVE_MOUNTED) && (nwDriveInfo[nwDriveIdx].nwDriveStatus != NETWORK_DRIVE_UNMOUNTING))
    {
        MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
        return;
    }
    MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);

    ReadScheduleBackupConfig(&scheduleBkupCfg);
    if ((scheduleBkupCfg.backupLocation == SB_TO_NETWORK_DRIVE_1) || (scheduleBkupCfg.backupLocation == SB_TO_NETWORK_DRIVE_2))
    {
        ExitBkUpCleanUpThread();
    }

    MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
    DPRINT(NDD_MANAGER, "unmount device: [nddMountPoint=%s]", nwDriveInfo[nwDriveIdx].nddMountPoint);
    if(unmountNetworkDevice(nwDriveInfo[nwDriveIdx].nddMountPoint) == FAIL)
    {
        EPRINT(NDD_MANAGER, "unmount fail: [ndd=%s]", nwMediaNameStr[nwDriveIdx]);
    }

    nwDriveInfo[nwDriveIdx].nwDriveStatus = NETWORK_DRIVE_DISCONNECTED;
    nwDriveInfo[nwDriveIdx].storageStatus.freeSize  = 0;
    nwDriveInfo[nwDriveIdx].storageStatus.totalSize = 0;
    nwDriveInfo[nwDriveIdx].storageStatus.usedSize = 0;
    nwDriveInfo[nwDriveIdx].nddMountPoint[0] = '\0';
    nwDriveInfo[nwDriveIdx].volStatus = DM_DISK_NOT_CONNECTED;
    nwDriveInfo[nwDriveIdx].healthStatus = STRG_HLT_ERROR;
    for(dirCnt = MATRIX_REC_DIR; dirCnt < MAX_MATRIX_DIR; dirCnt++)
    {
        nwDriveInfo[nwDriveIdx].mtxDirMountPoint[dirCnt][0] = '\0';
    }
    MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will mount the requested network drive as per configuration if enabled.
 * @param   nwDriveIdx
 * @return  SUCCESS/FAIL
 */
static BOOL makeNwDriveReadyToUse(UINT8 nwDriveIdx)
{
    NDD_MATRIX_DIR_e		matrixDir;
	CHAR					sysCmd[SYSTEM_COMMAND_SIZE];
	CHAR					nddMountPoint[MOUNT_POINT_SIZE];
	CHAR 					macAddress[MAX_MAC_ADDRESS_WIDTH];
    CHAR                    tUserName[MAX_NW_DRIVE_USER_NAME_LEN] = "";
    CHAR                    tDomain[MAX_NW_DRIVE_DOMAIN_NAME_LEN] = "";
	NETWORK_DRIVE_CONFIG_t	nwDriveCfg;
    UINT32                  tErrorCode = INVALID_ERROR_CODE;
    CHAR                    ipAddressForUrl[DOMAIN_NAME_SIZE_MAX] = { 0 };

	do
	{
		ReadSingleNetworkDriveConfig(nwDriveIdx, &nwDriveCfg);
        if(nwDriveCfg.enable == DISABLE)
		{
            break;
        }

        if(nwDriveCfg.fileSys == NFS_FILE_SYS)
        {
            PrepareIpAddressForUrl(nwDriveCfg.ipAddr, ipAddressForUrl);
            snprintf(sysCmd, SYSTEM_COMMAND_SIZE, MOUNT_NFS_DRIVE NETWORK_DRIVE_MOUNT_PATH, ipAddressForUrl, nwDriveCfg.dfltFolder, (nwDriveIdx + 1));
        }
        else if(nwDriveCfg.fileSys == CIFS_FILE_SYS)
        {
            if(FAIL == getDomainAndUserName(nwDriveCfg.userName, tDomain, tUserName))
            {
                EPRINT(NDD_MANAGER, "failed to get domain and username: [userName=%s]", nwDriveCfg.userName);
                break;
            }

            snprintf(sysCmd, SYSTEM_COMMAND_SIZE, MOUNT_CIFS_DRIVE, nwDriveCfg.ipAddr, nwDriveCfg.dfltFolder, "", (nwDriveIdx + 1));

            /* check username is present or not */
            if (tUserName[0] != '\0')
            {
                /* Append user name */
                snprintf(sysCmd + strlen(sysCmd), SYSTEM_COMMAND_SIZE - strlen(sysCmd), ",username=%s", tUserName);

                /* check password is present or not */
                if (nwDriveCfg.password[0] != '\0')
                {
                    /* Append user name */
                    snprintf(sysCmd + strlen(sysCmd), SYSTEM_COMMAND_SIZE - strlen(sysCmd), ",password=%s", nwDriveCfg.password);
                }
            }

            /* check domain is present or not */
            if (tDomain[0] != '\0')
            {
                /* append domain field in command */
                snprintf(sysCmd + strlen(sysCmd), SYSTEM_COMMAND_SIZE - strlen(sysCmd), ",domain=%s", tDomain);
            }
        }
        else
        {
            EPRINT(NDD_MANAGER, "invalid filesystem: [fileSys=%d], [media=%s]", nwDriveCfg.fileSys, nwMediaNameStr[nwDriveIdx]);
            break;
        }

        snprintf(nddMountPoint, MOUNT_POINT_SIZE, NETWORK_DRIVE_MOUNT_PATH, (nwDriveIdx + 1));
        if(CreateRecursiveDir(nddMountPoint, NULL) == FAIL)
        {
            EPRINT(NDD_MANAGER, "mount dir not created: [nddMountPoint=%s]", nddMountPoint);
            break;
        }

        GetMacAddrPrepare(LAN1_PORT, macAddress);
        if(FALSE == ExeSysCmd(TRUE, sysCmd))
        {
            EPRINT(NDD_MANAGER, "cmd fail: [cmd=%s]", sysCmd);
            if(SUCCESS == isDriveMounted(nddMountPoint))
            {
                /* Change status to drive unmounting so drive can be successfully unmounted */
                EPRINT(NDD_MANAGER, "network drive is already mounted: [ndd=%s]", nwMediaNameStr[nwDriveIdx]);
                MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
                nwDriveInfo[nwDriveIdx].nwDriveStatus = NETWORK_DRIVE_UNMOUNTING;
                snprintf(nwDriveInfo[nwDriveIdx].nddMountPoint, MOUNT_POINT_SIZE, "%s", nddMountPoint);
                MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);

                /* Drive is already mounted, Unmount the drive */
                disconnectFromNwDrive(nwDriveIdx);
            }
            else if(STATUS_OK != rmdir(nddMountPoint)) /* Remove mount point */
            {
                EPRINT(NDD_MANAGER, "fail to remove mount path: [nddMountPoint=%s], [err=%s]", nddMountPoint, STR_ERR);
            }
            break;
        }

        DPRINT(NDD_MANAGER, "cmd success: [cmd=%s]", sysCmd);
        if (FALSE == isDriveReadyToUse(nddMountPoint, &tErrorCode))
        {
            DPRINT(NDD_MANAGER, "ndd is not ready to use: [mountPoint=%s], [err=%s]", nddMountPoint, strerror(tErrorCode));
            unmountNetworkDevice(nddMountPoint);
            break;
        }

        // Prepare matrix directory structure
        DPRINT(NDD_MANAGER, "ndd is ready to use: [mountPoint=%s]", nddMountPoint);
        for(matrixDir = MATRIX_REC_DIR; matrixDir < MAX_MATRIX_DIR; matrixDir++)
        {
            snprintf(sysCmd, SYSTEM_COMMAND_SIZE, mtxDirStructure[matrixDir], nddMountPoint, macAddress);
            MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
            snprintf(nwDriveInfo[nwDriveIdx].mtxDirMountPoint[matrixDir], MOUNT_POINT_SIZE, "%s", sysCmd);
            MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
            if(CreateRecursiveDir(sysCmd, NULL) == FAIL)
            {
                EPRINT(NDD_MANAGER, "fail to create matrix dir: [cmd=%s], [ndd=%s]", sysCmd, nwMediaNameStr[nwDriveIdx]);
            }
        }

        MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
        snprintf(nwDriveInfo[nwDriveIdx].nddMountPoint, MOUNT_POINT_SIZE, "%s", nddMountPoint);
        MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
        if(VerifyHddSignature(nwDriveInfo[nwDriveIdx]. mtxDirMountPoint[MATRIX_REC_DIR]) == FAIL)
        {
            if(ReadyNetworkDevice(nwDriveIdx, NULL) == FAIL)
            {
                EPRINT(NDD_MANAGER, "formatting fail: [ndd=%s]", nwMediaNameStr[nwDriveIdx]);
                unmountNetworkDevice(nddMountPoint);
                break;
            }
        }

        MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
        nwDriveInfo[nwDriveIdx].nwDriveStatus = NETWORK_DRIVE_MOUNTED;
        nwDriveInfo[nwDriveIdx].volStatus = DM_DISK_VOL_NORMAL;
        nwDriveInfo[nwDriveIdx].healthStatus = STRG_HLT_DISK_NORMAL;
        snprintf(nwDriveInfo[nwDriveIdx].nddMountPoint, MOUNT_POINT_SIZE, "%s", nddMountPoint);
        MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
        return SUCCESS;

    } while(0);

    MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
    nwDriveInfo[nwDriveIdx].nwDriveStatus = NETWORK_DRIVE_DISCONNECTED;
    nwDriveInfo[nwDriveIdx].volStatus = DM_DISK_NOT_CONNECTED;
    nwDriveInfo[nwDriveIdx].healthStatus = STRG_HLT_ERROR;
    MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will find the used, free and available size of the mounted drive.
 * @param   nwDriveIdx
 * @return
 */
static BOOL getNwDriveStorageStatus(UINT8 nwDriveIdx)
{
	CHAR			mountPoint[MOUNT_POINT_SIZE];
	DISK_SIZE_t		diskSize = { 0 };
    struct stat     tFileInfoBuff;

    MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
    snprintf(mountPoint, MOUNT_POINT_SIZE, "%s", nwDriveInfo[nwDriveIdx].nddMountPoint);
    MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);

    if(STATUS_OK != stat(mountPoint, &tFileInfoBuff))
    {
        EPRINT(NDD_MANAGER, "fail to get ndd size, mount point not exist: [mountPoint=%s]", mountPoint);
        return FAIL;
    }

	if(SUCCESS != GetSizeOfMountFs(mountPoint, &diskSize))
	{
        EPRINT(NDD_MANAGER, "fail to get size of mount fs: [mountPoint=%s]", mountPoint);
		return FAIL;
	}

    MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
	nwDriveInfo[nwDriveIdx].storageStatus.freeSize = diskSize.freeSize;
	nwDriveInfo[nwDriveIdx].storageStatus.totalSize = diskSize.totalSize;
	nwDriveInfo[nwDriveIdx].storageStatus.usedSize = diskSize.usedSize;
    MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);

    /* Ideally there should not be mismatch, even if mismatch persisit it should not be huge */
	if(diskSize.totalSize != (diskSize.freeSize + diskSize.usedSize))
	{
        EPRINT(NDD_MANAGER, "nw disk size mismatch: [free=%d], [used=%d], [total=%d]", diskSize.freeSize, diskSize.usedSize,  diskSize.totalSize);
	}

	return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will take action based on the configuration change
 * @param   newNddConfig
 * @param   oldNddConfig
 * @param   nwDriveIndex
 * @return
 */
BOOL NetworkDriveConfigChangeNotify(NETWORK_DRIVE_CONFIG_t newNddConfig, NETWORK_DRIVE_CONFIG_t * oldNddConfig, UINT8 nwDriveIndex)
{
    MUTEX_LOCK(nddCfgChng.nasIdxMutex);
     if((oldNddConfig->enable == newNddConfig.enable) && (strcmp(oldNddConfig->dfltFolder, newNddConfig.dfltFolder) == STATUS_OK)
             && (oldNddConfig->fileSys == newNddConfig.fileSys) && (strcmp(oldNddConfig->ipAddr, newNddConfig.ipAddr) == STATUS_OK)
             && (strcmp(oldNddConfig->password, newNddConfig.password) == STATUS_OK) && (strcmp(oldNddConfig->userName, newNddConfig.userName) == STATUS_OK))
	 {
         MUTEX_UNLOCK(nddCfgChng.nasIdxMutex);
         return FAIL;
     }

    /* If nddIdx is max so it is free to take config change */
    if(nddCfgChng.nddIdx != MAX_NW_DRIVE)
    {
        MUTEX_UNLOCK(nddCfgChng.nasIdxMutex);
        return FAIL;
    }
    nddCfgChng.nddIdx = nwDriveIndex;
    MUTEX_UNLOCK(nddCfgChng.nasIdxMutex);

    memcpy(&nddCfgChng.oldCopy, oldNddConfig, sizeof(NETWORK_DRIVE_CONFIG_t));
    memcpy(&nddCfgChng.newCopy, &newNddConfig, sizeof(NETWORK_DRIVE_CONFIG_t));
    if (FALSE == Utils_CreateThread(NULL, updateNddConfig, NULL, DETACHED_THREAD, UPDATE_CONFIG_THREAD_STACK_SZ))
    {
        EPRINT(NDD_MANAGER, "fail to create ndd config change thread");
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread function and it takes action on changing network drive config.
 * @param   threadArg
 * @return
 */
static VOIDPTR updateNddConfig(VOIDPTR threadArg)
{
	UINT8						nddIndex;
    NETWORK_DRIVE_STATUS_e      volStatus;
	HDD_CONFIG_t				hddConfig;

	ReadHddConfig(&hddConfig);
    MUTEX_LOCK(nddCfgChng.nasIdxMutex);
	nddIndex = nddCfgChng.nddIdx;
    MUTEX_UNLOCK(nddCfgChng.nasIdxMutex);

	SetStorageDriveStatus(REMOTE_NAS1, DISK_ACT_CONFIG_CHANGE);
	SetStorageDriveStatus(REMOTE_NAS2, DISK_ACT_CONFIG_CHANGE);
    if((UINT8)hddConfig.recordDisk == (nddIndex + REMOTE_NAS1))
	{
        SetNddNonFuncStatus(TRUE, nddIndex);
		PreFormatDiskFunc(DISK_ACT_CONFIG_CHANGE);
	}
	else
	{
		STOP_NDD_FUNCTINALITY(nddIndex);
	}

    MUTEX_LOCK(nwDriveInfo[nddIndex].nwDriveInfoLock);
	nwDriveInfo[nddIndex].nwDriveStatus = NETWORK_DRIVE_UNMOUNTING;
    MUTEX_UNLOCK(nwDriveInfo[nddIndex].nwDriveInfoLock);

    /* Upload stop */
	sleep(2);

	if(nddCfgChng.oldCopy.enable != nddCfgChng.newCopy.enable)
	{
		if(nddCfgChng.newCopy.enable == ENABLE)
		{
            startNwDriveRetryTmr((NETWORK_DEVICE_TIMEOUT/6));
			volStatus = NETWORK_DRIVE_DISCONNECTED;
		}
		else
		{
			DeleteTimer(&nwDriveTmrHndl);
            disconnectFromNwDrive(nddIndex);
			volStatus = NETWORK_DRIVE_DISABLE;
		}

        MUTEX_LOCK(nwDriveInfo[nddIndex].nwDriveInfoLock);
		nwDriveInfo[nddIndex].nwDriveStatus = volStatus;
		nwDriveInfo[nddIndex].healthStatus = STRG_HLT_NO_DISK;
        MUTEX_UNLOCK(nwDriveInfo[nddIndex].nwDriveInfoLock);
	}
    else if((strcmp(nddCfgChng.oldCopy.dfltFolder, nddCfgChng.newCopy.dfltFolder) != STATUS_OK)
			|| (nddCfgChng.oldCopy.fileSys != nddCfgChng.newCopy.fileSys)
			|| (strcmp(nddCfgChng.oldCopy.ipAddr, nddCfgChng.newCopy.ipAddr) != STATUS_OK)
			|| (strcmp(nddCfgChng.oldCopy.password, nddCfgChng.newCopy.password) != STATUS_OK)
            || (strcmp(nddCfgChng.oldCopy.userName, nddCfgChng.newCopy.userName) != STATUS_OK))
	{
		DeleteTimer(&nwDriveTmrHndl);
        disconnectFromNwDrive(nddIndex);
        startNwDriveRetryTmr((NETWORK_DEVICE_TIMEOUT/6));
	}

    MUTEX_LOCK(nddCfgChng.nasIdxMutex);
	nddCfgChng.nddIdx = MAX_NW_DRIVE;
    MUTEX_UNLOCK(nddCfgChng.nasIdxMutex);
    DPRINT(NDD_MANAGER, "nas update thread exit");
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function unmount the device as from given mount point
 * @param   mountPath
 * @return  SUCCESS/FAIL
 */
static BOOL unmountNetworkDevice(CHARPTR mountPath)
{
    if(access(mountPath, F_OK) != STATUS_OK)
    {
        EPRINT(NDD_MANAGER, "mount point not present: [mountPath=%s]", mountPath);
        return FAIL;
    }

    if(umount(mountPath) != STATUS_OK)
    {
        if(umount2(mountPath, MNT_FORCE) != STATUS_OK)
        {
            EPRINT(NDD_MANAGER, "device not forcefully unmount: [mountPath=%s], [err=%s]", mountPath, STR_ERR);
            if(umount2(mountPath, MNT_DETACH) != STATUS_OK)
            {
                EPRINT(NDD_MANAGER, "device not detached: [mountPath=%s], [err=%s]", mountPath, STR_ERR);
                return FAIL;
            }
        }
    }

    if (rmdir(mountPath) != STATUS_OK)
    {
        EPRINT(NDD_MANAGER, "fail to remove mount path: [mountPath=%s], [err=%s]", mountPath, STR_ERR);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function removes all the data from given point.
 * @param   mntPoint
 * @return  SUCCESS/FAIL
 */
static BOOL formatNetworkDevice(CHARPTR	mntPoint)
{
	if(RemoveDirectory(mntPoint) == FAIL)
	{
        EPRINT(NDD_MANAGER, "fail to remove directory: [mntPoint=%s]", mntPoint);
	}

    if(CreateRecursiveDir(mntPoint, NULL) == FAIL)
	{
        DPRINT(NDD_MANAGER, "fail to create directory: [mntPoint=%s]", mntPoint);
        return FAIL;
	}

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function removes all the data from given device id and makes it useful for recording,
 *          backup and image Upload.
 * @param   nwDriveIdx
 * @param   advncDetail
 * @return  SUCCESS/FAIL
 */
BOOL ReadyNetworkDevice(UINT8 nwDriveIdx, CHARPTR advncDetail)
{
    CHAR mntPoint[MOUNT_POINT_SIZE];
    CHAR detail[MAX_EVENT_DETAIL_SIZE];

    if(nwDriveIdx >= MAX_NW_DRIVE)
	{
        return FAIL;
    }

    MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
    snprintf(mntPoint, MOUNT_POINT_SIZE, "%s", nwDriveInfo[nwDriveIdx].mtxDirMountPoint[MATRIX_REC_DIR]);
    nwDriveInfo[nwDriveIdx].nwDriveStatus = NETWORK_DRIVE_FORMATTING;
    nwDriveInfo[nwDriveIdx].volStatus = DM_DISK_VOL_FORMATTING;
    MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);

    snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", (nwDriveIdx + 21)); // raid vol spilted for event
    WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_VOLUME, detail, advncDetail, EVENT_FORMAT);
    formatNetworkDevice(mntPoint);
    if (getNwDriveStorageStatus(nwDriveIdx) == FAIL)
    {
        EPRINT(NDD_MANAGER, "size not available: [ndd=%s]", nwMediaNameStr[nwDriveIdx]);
        return FAIL;
    }

    if (WriteHddSignature(mntPoint) == FAIL)
    {
        EPRINT(NDD_MANAGER, "write signature fail: [ndd=%s]", nwMediaNameStr[nwDriveIdx]);
        return FAIL;
    }

    MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
    nwDriveInfo[nwDriveIdx].nwDriveStatus = NETWORK_DRIVE_MOUNTED;
    nwDriveInfo[nwDriveIdx].volStatus = DM_DISK_VOL_NORMAL;
    SetNddNonFuncStatus(FALSE,nwDriveIdx);
    MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives free, used and total size of disk.
 * @param   nddId
 * @param   diskSize
 * @return  SUCCESS/FAIL
 */
BOOL GetNasDriveSize(UINT8 nddId, DISK_SIZE_t *diskSize)
{
    if(nddId >= MAX_NW_DRIVE)
	{
        return FAIL;
    }

    MUTEX_LOCK(nwDriveInfo[nddId].nwDriveInfoLock);
    if(nwDriveInfo[nddId].nwDriveStatus != NETWORK_DRIVE_MOUNTED)
    {
        MUTEX_UNLOCK(nwDriveInfo[nddId].nwDriveInfoLock);
        return FAIL;
    }
    memcpy(diskSize, &nwDriveInfo[nddId].storageStatus, sizeof(DISK_SIZE_t));
    MUTEX_UNLOCK(nwDriveInfo[nddId].nwDriveInfoLock);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives mount point of given disk id,if disk mounted proper.
 * @param   diskId
 * @param   path
 * @return  SUCCESS/FAIL
 */
BOOL GetMountPointFromNetworkDiskId(UINT32 diskId, CHARPTR path)
{
    if(diskId >= MAX_NW_DRIVE)
	{
        return FAIL;
    }

    MUTEX_LOCK(nwDriveInfo[diskId].nwDriveInfoLock);
    if(nwDriveInfo[diskId].nwDriveStatus != NETWORK_DRIVE_MOUNTED)
    {
        MUTEX_UNLOCK(nwDriveInfo[diskId].nwDriveInfoLock);
        path[0] = '\0';
        return FAIL;
    }
    snprintf(path, MAX_FILE_NAME_SIZE, "%s", nwDriveInfo[diskId].mtxDirMountPoint[MATRIX_REC_DIR]);
    MUTEX_UNLOCK(nwDriveInfo[diskId].nwDriveInfoLock);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sets ndd function status
 * @param   recDiskId
 * @param   status
 */
void SetNddHealthStatus(UINT8 recDiskId, STORAGE_HEALTH_STATUS_e status)
{
    if(recDiskId >= MAX_NW_DRIVE)
	{
        return;
    }

    MUTEX_LOCK(nwDriveInfo[recDiskId].nwDriveInfoLock);
    nwDriveInfo[recDiskId].healthStatus = status;
    MUTEX_UNLOCK(nwDriveInfo[recDiskId].nwDriveInfoLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sets ndd volume status
 * @param   recordingDisk
 * @param   status
 */
void SetNddVolumeStatus(UINT8 recordingDisk, DISK_VOL_STATUS_e status)
{
    if(recordingDisk >= MAX_NW_DRIVE)
	{
        return;
    }

    MUTEX_LOCK(nwDriveInfo[recordingDisk].nwDriveInfoLock);
    nwDriveInfo[recordingDisk].volStatus = status;
    MUTEX_UNLOCK(nwDriveInfo[recordingDisk].nwDriveInfoLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives hard disk status in which recording is enable
 * @param   diskId
 * @return
 */
NET_CMD_STATUS_e GiveNddStatus(UINT8 diskId)
{
    if(diskId >= MAX_NW_DRIVE)
	{
        return CMD_PROCESS_ERROR;
    }

    MUTEX_LOCK(nwDriveInfo[diskId].nwDriveInfoLock);
    if((nwDriveInfo[diskId].nwDriveStatus == NETWORK_DRIVE_DISCONNECTED) || (nwDriveInfo[diskId].nwDriveStatus == NETWORK_DRIVE_DISABLE))
    {
        MUTEX_UNLOCK(nwDriveInfo[diskId].nwDriveInfoLock);
        return CMD_NO_DISK_FOUND;
    }

    if(nwDriveInfo[diskId].nwDriveStatus == NETWORK_DRIVE_FORMATTING)
    {
        MUTEX_UNLOCK(nwDriveInfo[diskId].nwDriveInfoLock);
        return CMD_FORMAT_IN_PROCESS;
    }

    if(nwDriveInfo[diskId].nwDriveStatus == NETWORK_DRIVE_MOUNTED)
    {
        MUTEX_UNLOCK(nwDriveInfo[diskId].nwDriveInfoLock);
        return CMD_SUCCESS;
    }

    MUTEX_UNLOCK(nwDriveInfo[diskId].nwDriveInfoLock);
    return CMD_PROCESS_ERROR;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives health status of hard disk.
 * @param   diskId
 * @return  status
 */
STORAGE_HEALTH_STATUS_e GetNddHealthStatus(UINT8 diskId)
{
    if(diskId >= MAX_NW_DRIVE)
	{
        return STRG_HLT_MAX;
    }

    MUTEX_LOCK(nwDriveInfo[diskId].nwDriveInfoLock);
    STORAGE_HEALTH_STATUS_e status = nwDriveInfo[diskId].healthStatus;
    MUTEX_UNLOCK(nwDriveInfo[diskId].nwDriveInfoLock);
	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used whenever user was request to get status of logical volume of disk.
 *          It will gives volume name, total size, free size and status as out put parameters.
 * @param   diskId
 * @param   diskVolumeInfo
 * @return  SUCCESS/FAIL
 */
BOOL GetNddVolumeInfo(UINT8 diskId, DISK_VOLUME_INFO_t *diskVolumeInfo)
{
    float hardiskSize;

    if((diskVolumeInfo == NULL) || (diskId >= MAX_NW_DRIVE))
	{
		EPRINT(DISK_MANAGER, "invalid parameter");
        return FAIL;
	}

    MUTEX_LOCK(nwDriveInfo[diskId].nwDriveInfoLock);
    if(nwDriveInfo[diskId].nwDriveStatus == NETWORK_DRIVE_DISABLE)
    {
        MUTEX_UNLOCK(nwDriveInfo[diskId].nwDriveInfoLock);
        return UNKNOWN;
    }

    snprintf(diskVolumeInfo->volumeName, MAX_VOLUME_NAME_SIZE, "%s", nwMediaNameStr[diskId]);
    hardiskSize = ((float)nwDriveInfo[diskId].storageStatus.totalSize / KILO_BYTE);
    snprintf(diskVolumeInfo->totalSize, MAX_HDD_CAPACITY + 1, "%.02f", hardiskSize);
    hardiskSize = ((float)nwDriveInfo[diskId].storageStatus.freeSize / KILO_BYTE);
    snprintf(diskVolumeInfo->freeSize, MAX_HDD_CAPACITY + 1, "%.02f", hardiskSize);
    diskVolumeInfo->status = nwDriveInfo[diskId].volStatus;
    snprintf(diskVolumeInfo->percentageFormat, MAX_PERCENTAGE_CHAR, "0.0");
    MUTEX_UNLOCK(nwDriveInfo[diskId].nwDriveInfoLock);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function check whether format device available for formatting or not.
 * @param   formatDevice
 * @return
 */
NET_CMD_STATUS_e CheckNddFormatDevice(UINT8 formatDevice)
{
    NETWORK_DRIVE_STATUS_e	status;
	HDD_CONFIG_t			hddConfig;

    if(formatDevice >= MAX_NW_DRIVE)
	{
        return CMD_PROCESS_ERROR;
    }

    MUTEX_LOCK(nwDriveInfo[formatDevice].nwDriveInfoLock);
    status = nwDriveInfo[formatDevice].nwDriveStatus;
    MUTEX_UNLOCK(nwDriveInfo[formatDevice].nwDriveInfoLock);
    if(status == NETWORK_DRIVE_DISCONNECTED)
    {
        return CMD_NO_DISK_FOUND;
    }

    if(GetNddVolBuildStatus() == TRUE)
    {
        return CMD_RESOURCE_LIMIT;
    }

    ReadHddConfig(&hddConfig);
    if((UINT8)hddConfig.recordDisk == (formatDevice + REMOTE_NAS1))
    {
        SetNddNonFuncStatus(TRUE, formatDevice);
    }
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function invokes whenever configuration was changed of Hard disk config.
 * @param   newHddConfig
 * @param   oldHddConfig
 * @return  SUCCESS/FAIL
 */
BOOL CheckNddHddCfgUpdate(HDD_CONFIG_t * newHddConfig, HDD_CONFIG_t * oldHddConfig)
{
    if((newHddConfig->recordDisk != oldHddConfig->recordDisk) && (newHddConfig->recordDisk != LOCAL_HARD_DISK))
	{
		if(GetNddVolBuildStatus() == FALSE)
		{
            return SUCCESS;
		}
	}

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function invokes whenever configuration was changed of Hard disk config.
 * @param   newHddConfig
 * @param   oldHddConfig
 * @return
 */
BOOL NddHddCfgUpdate(HDD_CONFIG_t * newHddConfig, HDD_CONFIG_t * oldHddConfig)
{
    UINT8 nwDriveIdx;

    if(GetNddVolBuildStatus() == TRUE)
	{
        return FAIL;
    }

    nwDriveIdx = (newHddConfig->recordDisk - REMOTE_NAS1);
    SetNddNonFuncStatus(TRUE, nwDriveIdx);
    SetStorageDriveStatus(ALL_DRIVE, DISK_ACT_CONFIG_CHANGE);
    PreFormatDiskFunc(DISK_ACT_CONFIG_CHANGE);

    MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
    if(nwDriveInfo[nwDriveIdx].nwDriveStatus == NETWORK_DRIVE_MOUNTED)
    {
        MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
        SetNddNonFuncStatus(FALSE,nwDriveIdx);
    }
    else
    {
        MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
        startNwDriveRetryTmr((NETWORK_DEVICE_TIMEOUT/6));
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initiates the uploading data to given NAS server. It will create a thread
 *          for this request and upload data is performed in this thread. Result of upload request
 *          is passed back to caller with callback.
 * @param   fileInfo
 * @param   userCallback
 * @param   userData
 * @param   nasHandle
 * @return  Status
 */
NET_CMD_STATUS_e StartNasUpload(NAS_FILE_INFO_t * fileInfo, NAS_CALLBACK userCallback, UINT16 userData, NAS_HANDLE * nasHandle)
{
	UINT8 					index;
	CHAR  					path[MOUNT_POINT_SIZE];
	NETWORK_DRIVE_CONFIG_t	nddConfig;

    if (nasHandle == NULL)
	{
        return CMD_PROCESS_ERROR;
    }

    *nasHandle = INVALID_NAS_HANDLE;
    if (fileInfo->nasServer >= MAX_NW_DRIVE)
	{
        return CMD_PROCESS_ERROR;
    }

    ReadSingleNetworkDriveConfig(fileInfo->nasServer, &nddConfig);
    if(nddConfig.enable == DISABLE)
    {
        return CMD_SERVER_DISABLED;
    }

    if(GetPathOfNetworkDrive(fileInfo->nasServer, MATRIX_IMG_DIR, path) == FAIL)
    {
        EPRINT(NDD_MANAGER, "nas server not ready: [addr=%d]", fileInfo->nasServer);
        return CMD_PROCESS_ERROR;
    }

    MUTEX_LOCK(nasReqMutex);
    for(index = 0; index < MAX_NAS_REQUEST; index++)
    {
        if(nasRequestList[index].requestStatus == FREE)
        {
            nasRequestList[index].requestStatus = BUSY;
            break;
        }
    }
    MUTEX_UNLOCK(nasReqMutex);

    if(index >= MAX_NAS_REQUEST)
    {
        EPRINT(NDD_MANAGER, "max nas session reached");
        return CMD_PROCESS_ERROR;
    }

    nasRequestList[index].nasReqIdx = index;
    memcpy(&nasRequestList[index].nasInfo, fileInfo, sizeof(NAS_FILE_INFO_t));
    nasRequestList[index].userCallback = userCallback;
    nasRequestList[index].userData = userData;
    if (FAIL == Utils_CreateThread(NULL, nasTransfer, &nasRequestList[index], DETACHED_THREAD, NAS_TRANSFER_THREAD_STACK_SZ))
    {
        EPRINT(NDD_MANAGER, "fail to create nas session thread");
        MUTEX_LOCK(nasReqMutex);
        nasRequestList[index].requestStatus = FREE;
        MUTEX_UNLOCK(nasReqMutex);
        return CMD_PROCESS_ERROR;
    }

    *nasHandle = index;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initiates the uploading data to given NAS server. It will create a thread
 *          for this request and upload data is performed in this thread. Result of upload request
 *          is passed back to caller with callback.
 * @param   fileInfo
 * @return  Status
 */
NET_CMD_STATUS_e NasImageUpload(NAS_FILE_INFO_t * fileInfo)
{
    if (fileInfo->nasServer >= MAX_NW_DRIVE)
	{
        return CMD_PROCESS_ERROR;
    }

    if (QueueAddEntry(nddImageUpld[fileInfo->nasServer].nasQHandle, fileInfo) == FAIL)
    {
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread function and it will copy image file to NAS server
 * @param   threadArg
 * @return
 */
static VOIDPTR nasTransfer(VOIDPTR threadArg)
{
	NAS_RESPONSE_e		response = NAS_FAIL;
    UINT8				index = ((NAS_REQUEST_LIST_t *)threadArg)->nasReqIdx;
	UINT32				length;
	CHAR				path[MOUNT_POINT_SIZE + 1];
	CHAR				remoteFileName[MAX_FILE_NAME_SIZE + 1];
	CHAR				dir[MAX_FILE_NAME_SIZE + 1];
    CHARPTR				strSearch;
	DISK_SIZE_t			diskSize;
	struct stat 		fileSize;

    THREAD_START_INDEX("NDD_XFER", index);
    do
    {
        if(GetPathOfNetworkDrive(nasRequestList[index].nasInfo.nasServer, MATRIX_IMG_DIR, path) == FAIL)
        {
            response = NAS_CONNECTION_ERROR;
            EPRINT(NDD_MANAGER, "connection error: [ndd=%s]", (nasRequestList[index].nasInfo.nasServer < MAX_NW_DRIVE) ? nwMediaNameStr[nasRequestList[index].nasInfo.nasServer] : "Invalid");
            break;
        }

        if((stat(nasRequestList[index].nasInfo.localFileName, &fileSize) != STATUS_OK)
                || (GetNasDriveSize(nasRequestList[index].nasInfo.nasServer, &diskSize) == FAIL))
        {
            response = NAS_SIZE_NOT_AVAILABLE;
            EPRINT(NDD_MANAGER, "file not found: [ndd=%s], [path=%s]", nwMediaNameStr[nasRequestList[index].nasInfo.nasServer], nasRequestList[index].nasInfo.localFileName);
            break;
        }

        if(diskSize.freeSize <= (UINT32)(fileSize.st_size / MEGA_BYTE))
        {
            response = NAS_SIZE_NOT_AVAILABLE;
            EPRINT(NDD_MANAGER, "size not available: [ndd=%s]", nwMediaNameStr[nasRequestList[index].nasInfo.nasServer]);
            break;
        }

        snprintf(remoteFileName, MAX_FILE_NAME_SIZE, "%s%s", path, nasRequestList[index].nasInfo.remoteFile);
        strSearch = strrchr(remoteFileName, '/');
        if(strSearch == NULL)
        {
            break;
        }

        length = (strlen(remoteFileName) - strlen(strSearch));
        if(length >= MAX_FILE_NAME_SIZE)
        {
            break;
        }

        snprintf(dir, length+1, "%s", remoteFileName);
        CreateRecursiveDir(dir, NULL);
        if(CopyFile(nasRequestList[index].nasInfo.localFileName, remoteFileName, 5000, NULL, 0) == FAIL)
        {
            break;
        }

        response = NAS_SUCCESS;

    }while(0);

	if(nasRequestList[index].userCallback != NULL)
	{
        nasRequestList[index].userCallback(index, response, nasRequestList[index].userData);
		nasRequestList[index].userCallback = NULL;
	}

    MUTEX_LOCK(nasReqMutex);
	nasRequestList[index].requestStatus = FREE;
    MUTEX_UNLOCK(nasReqMutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread function and it will copy image file to NAS server
 * @param   threadArg
 * @return
 */
static VOIDPTR nasImageUpload(VOIDPTR threadArg)
{
	NDD_IMAGE_UPLD_t	*nddImgUpldPtr = (NDD_IMAGE_UPLD_t *)threadArg;
	QUEUE_HANDLE		qHandle = nddImgUpldPtr->nasQHandle;
	NAS_RESPONSE_e		response;
	UINT32				length;
	CHAR				path[MOUNT_POINT_SIZE + 1];
	CHAR				remoteFileName[MAX_FILE_NAME_SIZE + 1];
	CHAR				dir[MAX_FILE_NAME_SIZE + 1];
    CHARPTR				strSearch;
	DISK_SIZE_t			diskSize;
	struct stat 		fileSize;
    NAS_FILE_INFO_t		*reqPtr = NULL;
    NAS_FILE_INFO_t		*getReqPtr = NULL;
    CHAR                eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR                advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    THREAD_START_INDEX("NDD_IMG_UPLD", nddImgUpldPtr->nddIdx);
    while (TRUE)
	{
		response = NAS_FAIL;
		if (reqPtr == NULL)
		{
			reqPtr = (NAS_FILE_INFO_t *)QueueGetAndFreeEntry(qHandle, TRUE);
		}
		else
		{
            getReqPtr = (NAS_FILE_INFO_t *)QueueGetNewEntryIfOverWritten(qHandle);
            if (getReqPtr != NULL)
			{
				unlink(reqPtr->localFileName);
				free(reqPtr);
                reqPtr = getReqPtr;
			}
		}

		if (reqPtr == NULL)
		{
			continue;
		}

        if (GetPathOfNetworkDrive(reqPtr->nasServer, MATRIX_IMG_DIR, path) == FAIL)
        {
            EPRINT(NDD_MANAGER, "connection error: [ndd=%s]", (reqPtr->nasServer < MAX_NW_DRIVE) ? nwMediaNameStr[reqPtr->nasServer] : "Invalid");
            sleep(NETWORK_DEVICE_TIMEOUT);
            continue;
        }

        do
        {
            if((stat(reqPtr->localFileName, &fileSize) != STATUS_OK) || (GetNasDriveSize(reqPtr->nasServer, &diskSize) == FAIL))
            {
                response = NAS_SIZE_NOT_AVAILABLE;
                EPRINT(NDD_MANAGER, "file not found: [ndd=%s], [path=%s]", nwMediaNameStr[reqPtr->nasServer], reqPtr->localFileName);
                break;
            }

            if(diskSize.freeSize <= (UINT32)(fileSize.st_size / MEGA_BYTE))
            {
                response = NAS_SIZE_NOT_AVAILABLE;
                EPRINT(NDD_MANAGER, "size not available: [ndd=%s]", nwMediaNameStr[reqPtr->nasServer]);
                break;
            }

            snprintf(remoteFileName, MAX_FILE_NAME_SIZE, "%s%s", path, reqPtr->remoteFile);
            strSearch = strrchr(remoteFileName, '/');
            if(strSearch == NULL)
            {
                break;
            }

            length = (strlen(remoteFileName) - strlen(strSearch));
            if(length >= MAX_FILE_NAME_SIZE)
            {
                break;
            }

            snprintf(dir, length+1, "%s", remoteFileName);
            CreateRecursiveDir(dir, NULL);
            if (CopyFile(reqPtr->localFileName, remoteFileName, 5000, NULL, 0) == FAIL)
            {
                break;
            }

            response = NAS_SUCCESS;

        }while(0);

        if (response != NAS_SUCCESS)
		{
            snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(reqPtr->camIndex));
            snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "NDD-%d | %s", reqPtr->nasServer+1, GetNddStatusStr(response));
            WriteEvent(LOG_NETWORK_EVENT, LOG_UPLOAD_IMAGE, eventDetail, advncDetail, EVENT_FAIL);
		}

		unlink(reqPtr->localFileName);
		free(reqPtr);
		reqPtr = NULL;
	}

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This callback is called when queue is full, and the entry is being over written
 * @param   entry
 */
static void queueFullCb(VOIDPTR entry)
{
	NAS_FILE_INFO_t	*reqPtr = (NAS_FILE_INFO_t *)entry;
    CHAR            eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR            advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(reqPtr->camIndex));
    snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "NDD-%d | %s", reqPtr->nasServer+1, GetNddStatusStr(NAS_QUEUE_FULL));
    WriteEvent(LOG_NETWORK_EVENT, LOG_UPLOAD_IMAGE, eventDetail, advncDetail, EVENT_FAIL);
	unlink(reqPtr->localFileName);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets path of Network drive location like Record, Backup, and for image upload.
 * @param   serverId
 * @param   location
 * @param   path
 * @return
 */
BOOL GetPathOfNetworkDrive(NAS_SERVER_e serverId, NDD_MATRIX_DIR_e location, CHARPTR path)
{
    if(serverId >= MAX_NW_DRIVE)
	{
        return FAIL;
    }

    MUTEX_LOCK(nwDriveInfo[serverId].nwDriveInfoLock);
    if(nwDriveInfo[serverId].nwDriveStatus != NETWORK_DRIVE_MOUNTED)
    {
        MUTEX_UNLOCK(nwDriveInfo[serverId].nwDriveInfoLock);
        return FAIL;
    }
    snprintf(path, MOUNT_POINT_SIZE, "%s", nwDriveInfo[serverId].mtxDirMountPoint[location]);
    MUTEX_UNLOCK(nwDriveInfo[serverId].nwDriveInfoLock);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets restarts network drive.
 * @param   hddConfigPtr
 */
void NddErrorHandle(HDD_CONFIG_t * hddConfigPtr)
{
    UINT8 driveIdx = (hddConfigPtr->recordDisk - REMOTE_NAS1);
	StopRecordDueToDiskFaultEvent(hddConfigPtr->recordDisk);
    disconnectFromNwDrive(driveIdx);
    startNwDriveRetryTmr(NETWORK_DEVICE_TIMEOUT);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates NAS storage device size.
 * @param   nddId
 */
void UpdateNddDiskSize(UINT8 nddId)
{
	if(nddId < MAX_NW_DRIVE)
	{
		getNwDriveStorageStatus(nddId);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks ndd connection is available or not
 * @param   nddIdx
 * @param   nddConfig
 * @param   callBack
 * @param   clientSocket
 * @return
 */
NET_CMD_STATUS_e NddTestConnection(UINT8 nddIdx, NETWORK_DRIVE_CONFIG_t *nddConfig, NW_CMD_REPLY_CB callBack, INT32 clientSocket)
{
    MUTEX_LOCK(nddTestConn.testNddMutex);
    if(nddTestConn.runningState == YES)
	{
        MUTEX_UNLOCK(nddTestConn.testNddMutex);
        return CMD_REQUEST_IN_PROGRESS;
    }

    nddTestConn.runningState = YES;
    nddTestConn.nddIdx = nddIdx;
    nddTestConn.callBack = callBack;
    nddTestConn.clientSocket = clientSocket;
    memcpy(&nddTestConn.nddConfig, nddConfig, sizeof(NETWORK_DRIVE_CONFIG_t));
    MUTEX_UNLOCK(nddTestConn.testNddMutex);
    DPRINT(NDD_MANAGER, "ndd test connection: [addr=%s], [path=%s]", nddTestConn.nddConfig.ipAddr, nddTestConn.nddConfig.dfltFolder);
    if (FALSE == Utils_CreateThread(NULL, connectTestNwDrive, NULL, DETACHED_THREAD, CONN_TEST_THREAD_STACK_SZ))
    {
        MUTEX_LOCK(nddTestConn.testNddMutex);
        nddTestConn.runningState = NO;
        MUTEX_UNLOCK(nddTestConn.testNddMutex);
        EPRINT(NDD_MANAGER, "fail to create ndd test connection thread");
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will do all the necessary setups to use the network drive. It will mount
 *          the drive and finds its storage status for further use.
 * @param   threadArg
 * @return
 */
static VOIDPTR connectTestNwDrive(VOIDPTR threadArg)
{
    NET_CMD_STATUS_e    cmdStatus = CMD_CONNECTIVITY_ERROR;
    CHAR                sysCmd[SYSTEM_COMMAND_SIZE];
    CHAR                nddMountPoint[MOUNT_POINT_SIZE];
    CHAR                tUserName[MAX_NW_DRIVE_USER_NAME_LEN] = "\0";
    CHAR                tDomain[MAX_NW_DRIVE_DOMAIN_NAME_LEN] = "\0";
    UINT32              tErrorCode = INVALID_ERROR_CODE;
    CHAR                ipAddressForUrl[DOMAIN_NAME_SIZE_MAX] = { 0 };

    do
	{
		if(nddTestConn.nddConfig.fileSys == NFS_FILE_SYS)
        {
            PrepareIpAddressForUrl(nddTestConn.nddConfig.ipAddr, ipAddressForUrl);
            snprintf(sysCmd, SYSTEM_COMMAND_SIZE, MOUNT_NFS_DRIVE NETWORK_TEST_DRIVE_MOUNT_PATH,
                     ipAddressForUrl, nddTestConn.nddConfig.dfltFolder, (nddTestConn.nddIdx + 1));
		}
		else if(nddTestConn.nddConfig.fileSys == CIFS_FILE_SYS)
		{
            if(FAIL == getDomainAndUserName(nddTestConn.nddConfig.userName, tDomain, tUserName))
            {
                EPRINT(NDD_MANAGER, "failed to get domain and username");
                break;
            }

            snprintf(sysCmd, SYSTEM_COMMAND_SIZE, MOUNT_CIFS_DRIVE, nddTestConn.nddConfig.ipAddr, nddTestConn.nddConfig.dfltFolder, "TEST_", (nddTestConn.nddIdx + 1));

            /* check username is present or not */
            if (tUserName[0] != '\0')
            {
                /* Append user name */
                snprintf(sysCmd + strlen(sysCmd), SYSTEM_COMMAND_SIZE - strlen(sysCmd), ",username=%s", tUserName);

                /* check password is present or not */
                if (nddTestConn.nddConfig.password[0] != '\0')
                {
                    /* Append user name */
                    snprintf(sysCmd + strlen(sysCmd), SYSTEM_COMMAND_SIZE - strlen(sysCmd), ",password=%s", nddTestConn.nddConfig.password);
                }
            }

            /* check domain is present or not */
            if (tDomain[0] != '\0')
            {
                /* append domain field in command */
                snprintf(sysCmd + strlen(sysCmd), SYSTEM_COMMAND_SIZE - strlen(sysCmd), ",domain=%s", tDomain);
            }
		}
		else
		{
            EPRINT(NDD_MANAGER, "invalid filesystem: [fileSys=%d], [ndd=%s]", nddTestConn.nddConfig.fileSys, nwMediaNameStr[nddTestConn.nddIdx]);
			break;
		}

        snprintf(nddMountPoint, MOUNT_POINT_SIZE, NETWORK_TEST_DRIVE_MOUNT_PATH, (nddTestConn.nddIdx + 1));
        if(CreateRecursiveDir(nddMountPoint, NULL) == FAIL)
		{
            EPRINT(NDD_MANAGER, "mount dir not created: [nddMountPoint=%s]", nddMountPoint);
			break;
		}

        if (FALSE == ExeSysCmd(TRUE, sysCmd))
        {
            rmdir(nddMountPoint);
            EPRINT(NDD_MANAGER, "cmd fail: [cmd=%s]", sysCmd);
            break;
        }

        if (FALSE == isDriveReadyToUse(nddMountPoint, &tErrorCode))
        {
            EPRINT(NDD_MANAGER, "test ndd fail: [mountPoint=%s], [err=%s]", nddMountPoint, strerror(tErrorCode));
            unmountNetworkDevice(nddMountPoint);
            break;
        }

        unmountNetworkDevice(nddMountPoint);
		cmdStatus = CMD_SUCCESS;

	}while(0);

    MUTEX_LOCK(nddTestConn.testNddMutex);
	nddTestConn.runningState = NO;
    MUTEX_UNLOCK(nddTestConn.testNddMutex);
	nddTestConn.callBack(cmdStatus, nddTestConn.clientSocket, TRUE);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives mount point of given disk id,if disk mounted proper.
 * @param   diskId
 * @param   path
 * @return  SUCCESS/FAIL
 */
BOOL GetMountPointFromNetworkDiskIdForBackUp(UINT32 diskId, CHARPTR path)
{
    if(diskId >= MAX_NW_DRIVE)
	{
        return FAIL;
    }

    MUTEX_LOCK(nwDriveInfo[diskId].nwDriveInfoLock);
    if(nwDriveInfo[diskId].nwDriveStatus != NETWORK_DRIVE_MOUNTED)
    {
        MUTEX_UNLOCK(nwDriveInfo[diskId].nwDriveInfoLock);
        path[0] = '\0';
        return FAIL;

    }
    snprintf(path, MOUNT_POINT_SIZE, "%s", nwDriveInfo[diskId].mtxDirMountPoint[MATRIX_BACKUP_DIR]);
    MUTEX_UNLOCK(nwDriveInfo[diskId].nwDriveInfoLock);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will do all the necessary setups to use the network drive. It will mount
 *          the drive and finds its storage status for further use.
 * @param   threadArg
 * @return
 */
static VOIDPTR checkNddMountStatus(VOIDPTR threadArg)
{
    UINT8						nwDriveIdx = 0;
	NETWORK_DRIVE_CONFIG_t		nddConfig;
	CHAR						recordFldr[SYSTEM_COMMAND_SIZE];
	CHAR						nddMntPoint[MOUNT_POINT_SIZE];
	CHAR 						macAddress[MAX_MAC_ADDRESS_WIDTH];
    BOOL                        removeMountDirF;
    struct stat                 statInfo;
    HDD_CONFIG_t                hddCnfg;
    CHAR                        sysCmd[SYSTEM_COMMAND_SIZE];

    /* /matrix/media/NDD1/00_05_d7_09_7a_03/SATATYA_RECORD */
    THREAD_START("NDD_MOUNT_STS");

    while(TRUE)
	{
        /* exit from thread on deinit */
		if(comeBackNddUpdatedThread == FALSE)
		{
			break;
		}

		for(nwDriveIdx = 0; nwDriveIdx < MAX_NW_DRIVE; nwDriveIdx++)
		{
			ReadSingleNetworkDriveConfig(nwDriveIdx, &nddConfig);
            if(nddConfig.enable == DISABLE)
			{
                MUTEX_LOCK(nddStatus);
                nddUpdateStatus[nwDriveIdx] = FALSE;
                MUTEX_UNLOCK(nddStatus);
                continue;
            }

            removeMountDirF = FALSE;
            GetMacAddrPrepare(LAN1_PORT, macAddress);
            snprintf(nddMntPoint, MOUNT_POINT_SIZE, NETWORK_DRIVE_MOUNT_PATH, (nwDriveIdx + 1));
            snprintf(recordFldr, SYSTEM_COMMAND_SIZE, MAKE_MATRIX_REC_DIR, nddMntPoint, macAddress);

            /* Note: Earlier for directory presence access api is used but in case of network drive disconnected
             * access API gives wrong results, Tried to to replace the access API with opendir() but that also
             * gives success even when network drive is disconnected, stat api fails when network drive is disconnected */
            if(STATUS_OK == stat(recordFldr, &statInfo))
            {
                MUTEX_LOCK(nddStatus);
                nddUpdateStatus[nwDriveIdx] = TRUE;
                MUTEX_UNLOCK(nddStatus);

                /* update ndd storage status */
                getNwDriveStorageStatus(nwDriveIdx);

                /* Check data is written into the flash or not */
                if (FAIL != isDriveMounted(nddMntPoint))
                {
                    /* Drive is mounted no need to do anything */
                    continue;
                }

                removeMountDirF = TRUE;
                EPRINT(NDD_MANAGER, "removing mount point as drive is unmounted: [nddMntPoint=%s]", nddMntPoint);
            }
            else
            {
                MUTEX_LOCK(nddStatus);
                nddUpdateStatus[nwDriveIdx] = FALSE;
                MUTEX_UNLOCK(nddStatus);

                MUTEX_LOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
                if(NETWORK_DRIVE_MOUNTED != nwDriveInfo[nwDriveIdx].nwDriveStatus)
                {
                    /* No need to do anything when drive status is not mounted */
                    MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
                    continue;
                }
                MUTEX_UNLOCK(nwDriveInfo[nwDriveIdx].nwDriveInfoLock);
            }

            /* Stop operations on disk due to NAS connectivity is lost or NAS status is incorrect */
            ReadHddConfig(&hddCnfg);
            SetNddNonFuncStatus(TRUE, nwDriveIdx);

            /* Check recording is ON on this drive or not */
            if((UINT8)hddCnfg.recordDisk == (nwDriveIdx + REMOTE_NAS1))
            {
                PreFormatDiskFunc(DISK_ACT_MAX);
            }
            else
            {
                STOP_NDD_FUNCTINALITY(nwDriveIdx);
            }

            /* Remove mount point start */
            if(removeMountDirF)
            {
                if(STATUS_OK != rmdir(nddMntPoint))
                {
                    /* Failed to remove directory may be some data is present in the directory */
                    snprintf(sysCmd, SYSTEM_COMMAND_SIZE, "rm -rf %s", nddMntPoint);
                    if (FALSE == ExeSysCmd(TRUE, sysCmd))
                    {
                        EPRINT(NDD_MANAGER, "fail to remove mount path: [nddMntPoint=%s], [err=%s]", nddMntPoint, STR_ERR);
                    }
                }
            }

            /* Disconnect from network drive and start the retry timer */
            DeleteTimer(&nwDriveTmrHndl);
            disconnectFromNwDrive(nwDriveIdx);
            startNwDriveRetryTmr(NETWORK_DEVICE_TIMEOUT);
		}

        /* Is ndd size update notify pending? */
        if (TRUE == nddSizeUpdateNotifyF)
        {
            nddSizeUpdateNotifyF = FALSE;
            NddSizeUpdateNotify();
        }

        MUTEX_LOCK(condWaitForNddStatus);
        pthread_cond_wait(&waitForNddStatus, &condWaitForNddStatus);
        MUTEX_UNLOCK(condWaitForNddStatus);
	}

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives status of Network drive.
 * @param   diskId
 * @return  status
 */
BOOL GetUpdatedNddStatus(UINT8 diskId)
{
    if(diskId >= MAX_NW_DRIVE)
	{
        return FALSE;
    }

    MUTEX_LOCK(nddStatus);
    BOOL status = nddUpdateStatus[diskId];
    MUTEX_UNLOCK(nddStatus);
	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function callback after every 60 seconds of time Interval
 * @param   data
 */
static void nddStatusUpdateTmrCb(UINT32 data)
{
    MUTEX_LOCK(condWaitForNddStatus);
	pthread_cond_signal(&waitForNddStatus);
    MUTEX_UNLOCK(condWaitForNddStatus);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function force to update NDD status
 */
void UpdateNddStorageStatus(void)
{
    /* Notify to disk controller once ndd size gets updated */
    nddSizeUpdateNotifyF = TRUE;
    MUTEX_LOCK(condWaitForNddStatus);
    pthread_cond_signal(&waitForNddStatus);
    MUTEX_UNLOCK(condWaitForNddStatus);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   get the current network drive status of the passed disk.
 * @param   iDiskId
 * @return  Network drive status
 */
NETWORK_DRIVE_STATUS_e GetNwDriveStatus(UINT8 iDiskId)
{
    if(iDiskId >= MAX_NW_DRIVE)
    {
        return MAX_NETWORK_DRIVE_STATUS;
    }

    MUTEX_LOCK(nwDriveInfo[iDiskId].nwDriveInfoLock);
    NETWORK_DRIVE_STATUS_e tNddStatus = nwDriveInfo[iDiskId].nwDriveStatus;
    MUTEX_UNLOCK(nwDriveInfo[iDiskId].nwDriveInfoLock);
    return tNddStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function parses the user entered username and get domain & username from it.
 * @param   pRcvdUserName
 * @param   pDomain
 * @param   pUserName
 * @return  SUCCESS/FAIL
 */
static BOOL getDomainAndUserName(CHARPTR pRcvdUserName, CHARPTR pDomain, CHARPTR pUserName)
{
    if((NULL == pRcvdUserName) || (NULL == pDomain) || (NULL == pUserName))
    {
        return FAIL;
    }

    /* username with domain - AD\admin (domain=AD and username=admin)
     * username without domain - admin (username=admin) */
    /* check domain is present or not */
    CHAR *pDomainSearch = strchr(pRcvdUserName, '\\' );
    if((NULL != pDomainSearch) && (pRcvdUserName[0] != '\0'))
    {
        /* extract domain name from the username */
        snprintf(pDomain, strlen(pRcvdUserName) - strlen(pDomainSearch) + 1, "%s", pRcvdUserName);

        /* remove domain name from the user name, + 1 is done for removing '\' */
        snprintf(pUserName, MAX_NW_DRIVE_USER_NAME_LEN, "%s", (pRcvdUserName + strlen(pDomain) + 1));
    }
    else /* domain is not present */
    {
        /* add user name  */
        snprintf(pUserName, MAX_NW_DRIVE_USER_NAME_LEN, "%s", pRcvdUserName);

        /* there is no domain present in the username */
        pDomain[0] = '\0';
    }

    DPRINT(NDD_MANAGER, "[cnfgUserName=%s], [domain=%s], [username=%s]", pRcvdUserName, pDomain, pUserName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get NDD status in string
 * @param   nddStatus
 * @return  NDD status in string format
 */
const CHARPTR GetNddStatusStr(NAS_RESPONSE_e nddStatus)
{
    switch(nddStatus)
    {
        case NAS_SUCCESS:
            return "Success";

        case NAS_CONNECTION_ERROR:
            return "Connection error";

        case NAS_SIZE_NOT_AVAILABLE:
            return "NDD size not available";

        case NAS_QUEUE_FULL:
            return "Internal queue full";

        case NAS_FAIL:
        default:
            return "Unknown";
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Test Read/Write Operation on given drive. Drive must be mounted before this.
 * @param   mntPoint
 * @param   pErrorCode
 * @return  Return TRUE if success else FALSE
 */
static BOOL isDriveReadyToUse(CHARPTR mntPoint, UINT32PTR pErrorCode)
{
    BOOL        status = FALSE;
    CHAR        sysCmd[SYSTEM_COMMAND_SIZE];
    CHAR        macAddress[MAX_MAC_ADDRESS_WIDTH] = { 0 };
    CHAR        fileName[MAX_FILE_NAME_SIZE];
    INT32       fileFd = INVALID_FILE_FD;
    struct tm   brokenTime = { 0 };

    do
    {
        GetMacAddrPrepare(LAN1_PORT, macAddress);
        snprintf(sysCmd, SYSTEM_COMMAND_SIZE, MAKE_MATRIX_TEST_DIR, mntPoint, macAddress);

        /* create test directory */
        if(CreateRecursiveDir(sysCmd, pErrorCode) == FAIL)
        {
            EPRINT(NDD_MANAGER, "fail to create test dir: [path=%s], [err=%s]", sysCmd, strerror(*pErrorCode));
            break;
        }

        GetLocalTimeInBrokenTm(&brokenTime);

        /* create test file path */
        snprintf(fileName, MAX_FILE_NAME_SIZE, "%s"TEST_FILE_NAME, sysCmd, brokenTime.tm_mday,
                 (brokenTime.tm_mon + 1), brokenTime.tm_year, brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec);

        /* create test file */
        fileFd = open(fileName, CREATE_RDWR_MODE, FILE_PERMISSION);
        if (fileFd == INVALID_FILE_FD)
        {
            SET_ERROR_NUMBER(pErrorCode, errno)
            EPRINT(NDD_MANAGER, "fail to open test file: [file=%s], [err=%s]", fileName, strerror(*pErrorCode));
            break;
        }

        /* write test msg */
        if (Utils_Write(fileFd, TEST_FILE_MESSAGE, strlen(TEST_FILE_MESSAGE), pErrorCode) != strlen(TEST_FILE_MESSAGE))
        {
            EPRINT(NDD_MANAGER, "fail to write test file: [file=%s], [err=%s]", fileName, strerror(*pErrorCode));
            break;
        }

        /* Close write fd */
        close(fileFd);

		/* open test file */
        fileFd = open(fileName, CREATE_RDWR_MODE, FILE_PERMISSION);
        if (fileFd == INVALID_FILE_FD)
        {
            SET_ERROR_NUMBER(pErrorCode, errno)
            EPRINT(NDD_MANAGER, "fail to open test file: [file=%s], [err=%s]", fileName, strerror(*pErrorCode));
            break;
        }

        /* read test msg */
        CHAR readBuf[100];
        if (Utils_Read(fileFd, readBuf, strlen(TEST_FILE_MESSAGE), pErrorCode) != strlen(TEST_FILE_MESSAGE))
        {
            EPRINT(NDD_MANAGER, "fail to read test file: [file=%s], [err=%s]", fileName, strerror(*pErrorCode));
            break;
        }

        /* Successfully read-write test file */
        status = TRUE;

    }while(0);

    CloseFileFd(&fileFd);

    /* remove test file */
    unlink(fileName);

    /* remove test directory */
    snprintf(sysCmd, SYSTEM_COMMAND_SIZE, "rm -rf "MAKE_MATRIX_TEST_DIR, mntPoint, macAddress);
    ExeSysCmd(TRUE, sysCmd);
    return status;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
