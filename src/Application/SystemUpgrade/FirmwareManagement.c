//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   FirmwareManagement.c
@brief  Firmware management module provides API to check for any upgrades available or not based
        on configuration. If it is available then it takes appropriate configured actions.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "FirmwareManagement.h"
#include "NetworkManager.h"
#include "NetworkCommand.h"
#include "EventHandler.h"
#include "DateTime.h"
#include "FtpClient.h"
#include "DebugLog.h"
#include "Utils.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define FIRMWARE_DOWNLOAD_INIT_TMR_IN_SEC       1
#define FIRMWARE_DOWNLOAD_RETRY_TMR_IN_SEC      300
#define FIRMWARE_DOWNLOAD_RETRY_MAX             3

#define FOLDER_VER_REV_FORMAT_STR               "V%hdR%hd"
#define FOLDER_VER_REV_SUB_REV_FORMAT_STR       FOLDER_VER_REV_FORMAT_STR".%hd"
#define FIRMWARE_VER_REV_FORMAT_STR             "V%02hdR%02hd"
#define FIRMWARE_VER_REV_SUB_REV_FORMAT_STR     FIRMWARE_VER_REV_FORMAT_STR".%hd"
#define VER_REV_STR_LEN_MIN                     4
#define FIRMWARE_RESP_STR                       respStatusStr[firmwareUpgradeInfo.firmwareResp]

#define FIRMWARE_DOWNLOAD_THREAD_STACK_SZ       (500 * KILO_BYTE)
#define CHECK_UPDATE_THREAD_STACK_SZ            (1 * MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    FIRMWARE_UPGRADE_RESP_OK = 0,
    FIRMWARE_UPGRADE_RESP_CONNECT_ERROR,
    FIRMWARE_UPGRADE_RESP_AUTH_ERROR,
    FIRMWARE_UPGRADE_RESP_FIRMWARE_NOT_FOUND,
    FIRMWARE_UPGRADE_RESP_FIRMWARE_UP_TO_DATE,
    FIRMWARE_UPGRADE_RESP_UNKNOWN,
    FIRMWARE_UPGRADE_RESP_MAX,

}FirmwareUpgradeResp_e;

typedef enum
{
    FIRMWARE_UPGRADE_TYPE_AUTO_NOTIFY = 0,
    FIRMWARE_UPGRADE_TYPE_AUTO_UPGRADE,
    FIRMWARE_UPGRADE_TYPE_MANAUL,
    FIRMWARE_UPGRADE_TYPE_MAX,

}FirmwareUpgradeType_e;

typedef enum
{
    FTP_DIR_LEVEL_ONE,      // Where you find Directories like V7R10, V7R11, etc..
    FTP_DIR_LEVEL_TWO,      // Where you find Directories like usb, web etc..
    FTP_DIR_LEVEL_THREE,    // Where you find Files like firmware.zip
    MAX_FTP_DIR_LEVEL,

}FTP_DIR_LEVEL_e;

typedef struct
{
    BOOL                    isProcessRunning;
    pthread_mutex_t         threadLock;
    FirmwareUpgradeType_e   upgradeType;
    FTP_DIR_LEVEL_e         dirLevel;
    FTP_SYS_e               ftpSysType;
    FirmwareUpgradeResp_e   firmwareResp;
    CLIENT_CB_TYPE_e        clientCbType;
    INT32                   clientSocket;
    CHAR                    latestVersion[DEVICE_SW_VERSION_LEN_MAX];
    CHAR                    firmwarePath[FTP_REMOTE_PATH_LEN];
    FTP_UPLOAD_CONFIG_t     ftpServerInfo;
    UINT8                   firmwareDownloadRetry;
    pthread_mutex_t         timerLock;
    TIMER_HANDLE            timerHandle;
    TIMER_HANDLE            eventTimerHandle;

}FIRMWARE_UPGRADE_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static FIRMWARE_UPGRADE_INFO_t  firmwareUpgradeInfo;

static NET_CMD_STATUS_e respStatusCode[FIRMWARE_UPGRADE_RESP_MAX] =
{
    CMD_SUCCESS,
    CMD_CONNECTIVITY_ERROR,
    CMD_FTP_INVALID_CREDENTIAL,
    CMD_FIRMWARE_NOT_FOUND,
    CMD_FIRMWARE_UP_TO_DATE,
    CMD_TESTING_FAIL,
};

static CHAR *respStatusStr[FIRMWARE_UPGRADE_RESP_MAX] =
{
    "OK",
    "CONNECT_ERROR",
    "AUTH_ERROR",
    "FIRMWARE_NOT_FOUND",
    "FIRMWARE_UP_TO_DATE",
    "UNKNOWN",
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void resetFirmwareUpgradeInfo(void);
//-------------------------------------------------------------------------------------------------
static BOOL checkScheduleFirmwareUpgrade(UINT32 data);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e checkFirmwareUpdate(FirmwareUpgradeType_e upgradeType, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket);
//-------------------------------------------------------------------------------------------------
static void *checkFirmwareUpdateThread(void *pThreadArg);
//-------------------------------------------------------------------------------------------------
static BOOL ftpDirListCb(CHARPTR dirList, UINT32 size, VOIDPTR userData);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e downloadAndUpgradeFirmware(CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket);
//-------------------------------------------------------------------------------------------------
static void *firmwareDownloadThread(void *pThreadArg);
//-------------------------------------------------------------------------------------------------
static void ftpFirmwareDownloadCb(FTP_HANDLE ftpHandle, FTP_RESPONSE_e ftpResp, UINT16 userData);
//-------------------------------------------------------------------------------------------------
static void startFirmwareDownloadTimer(UINT16 timeInSec);
//-------------------------------------------------------------------------------------------------
static void stopFirmwareDownloadTimer(void);
//-------------------------------------------------------------------------------------------------
static void firmwareDownloadTimerCb(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL isFirmwareDownloadRetryDone(FTP_RESPONSE_e ftpResp);
//-------------------------------------------------------------------------------------------------
static void startEventInactiveTimer(void);
//-------------------------------------------------------------------------------------------------
static void eventInactiveTimerCb(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void writeEventLog(FirmwareUpgradeResp_e firmwareResp);
//-------------------------------------------------------------------------------------------------
static void setFirmwareRespCode(FTP_RESPONSE_e ftpResp);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initialization of firmware management module
 */
void InitFirmwareManagement(void)
{
    ONE_MIN_NOTIFY_t oneMinuteTask;

    /* Init variables with default values */
    MUTEX_INIT(firmwareUpgradeInfo.threadLock, NULL);
    resetFirmwareUpgradeInfo();
    RESET_STR_BUFF(firmwareUpgradeInfo.firmwarePath);
    firmwareUpgradeInfo.upgradeType = FIRMWARE_UPGRADE_TYPE_MAX;
    memset(&firmwareUpgradeInfo.ftpServerInfo, 0, sizeof(firmwareUpgradeInfo.ftpServerInfo));
    MUTEX_INIT(firmwareUpgradeInfo.timerLock, NULL);
    firmwareUpgradeInfo.timerHandle = INVALID_TIMER_HANDLE;
    firmwareUpgradeInfo.eventTimerHandle = INVALID_TIMER_HANDLE;
    firmwareUpgradeInfo.firmwareDownloadRetry = 0;

    /* Register one minute task for schedule firmware upgrade */
    oneMinuteTask.funcPtr = checkScheduleFirmwareUpgrade;
    oneMinuteTask.userData = 0;
    if(RegisterOnMinFun(&oneMinuteTask) != SUCCESS)
    {
        EPRINT(SYSTEM_UPGRADE, "fail to start firmware upgrade schedule task");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   De-InitInitialization of firmware management module
 */
void DeInitFirmwareManagement(void)
{
    /* Stop firmware download retry timer if running */
    stopFirmwareDownloadTimer();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Change in firmware management config
 * @param   newCopy
 * @param   oldCopy
 */
void FirmwareManagementConfigNotify(FIRMWARE_MANAGEMENT_CONFIG_t newCopy, FIRMWARE_MANAGEMENT_CONFIG_t *oldCopy)
{
    /* Stop firmware download retry timer if running */
    stopFirmwareDownloadTimer();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check for manual firmware update
 * @param   clientCbType
 * @param   clientSocket
 * @return
 */
NET_CMD_STATUS_e CheckManualFirmwareUpdate(CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket)
{
    /* Check for firmware upgrade and provide a response */
    return checkFirmwareUpdate(FIRMWARE_UPGRADE_TYPE_MANAUL, clientCbType, clientSocket);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   UpgradeSystemFirmware
 * @param   clientCbType
 * @param   clientSocket
 * @return
 */
NET_CMD_STATUS_e UpgradeSystemFirmware(CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket)
{
    /* Download and upgrade the firmware */
    return downloadAndUpgradeFirmware(clientCbType, clientSocket);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Reset firmware upgrade information
 */
static void resetFirmwareUpgradeInfo(void)
{
    /* Reset firmware upgrade info variables */
    MUTEX_LOCK(firmwareUpgradeInfo.threadLock);
    firmwareUpgradeInfo.isProcessRunning = FALSE;
    firmwareUpgradeInfo.dirLevel = FTP_DIR_LEVEL_ONE;
    firmwareUpgradeInfo.ftpSysType = MAX_FTP_SYS;
    firmwareUpgradeInfo.firmwareResp = FIRMWARE_UPGRADE_RESP_OK;
    firmwareUpgradeInfo.clientCbType = CLIENT_CB_TYPE_MAX;
    firmwareUpgradeInfo.clientSocket = INVALID_CONNECTION;
    RESET_STR_BUFF(firmwareUpgradeInfo.latestVersion);
    MUTEX_UNLOCK(firmwareUpgradeInfo.threadLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   One minute time schedular callback for firmware upgrade check.
 * @param   data
 * @return  SUCCESS/FAIL
 */
static BOOL checkScheduleFirmwareUpgrade(UINT32 data)
{
    FIRMWARE_MANAGEMENT_CONFIG_t firmwareManageCfg;

    /* Read config and check firmware mode */
    ReadFirmwareManagementConfig(&firmwareManageCfg);
    if (firmwareManageCfg.autoUpgradeMode == AUTO_FIRMWARE_UPGRADE_NEVER)
    {
        /* Auto firmware upgrade disabled */
        return SUCCESS;
    }

    /* Get current time in broken time */
    struct tm brokenTime = {0};
    if (SUCCESS != GetLocalTimeInBrokenTm(&brokenTime))
    {
        EPRINT(SYSTEM_UPGRADE, "failed to get local time in broken");
        return FAIL;
    }

    /* Check configured hour with current hour */
    if (firmwareManageCfg.scheduleTime.hour != brokenTime.tm_hour)
    {
        /* Hour is different */
        return SUCCESS;
    }

    /* Check configured minute with current minute */
    if (firmwareManageCfg.scheduleTime.minute != brokenTime.tm_min)
    {
        /* Minute is different */
        return SUCCESS;
    }

    /* Check firmware update available or not */
    if (CMD_SUCCESS != checkFirmwareUpdate((firmwareManageCfg.autoUpgradeMode == AUTO_FIRMWARE_UPGRADE_NOTIFY_ONLY) ?
                                           FIRMWARE_UPGRADE_TYPE_AUTO_NOTIFY : FIRMWARE_UPGRADE_TYPE_AUTO_UPGRADE,
                                           CLIENT_CB_TYPE_MAX, INVALID_CONNECTION))
    {
        /* Failed to start firmware update check process */
        EPRINT(SYSTEM_UPGRADE, "firmware upgrade schedule check failed");
        writeEventLog(FIRMWARE_UPGRADE_RESP_UNKNOWN);
        return FAIL;
    }

    /* Firmware update schedule check started */
    DPRINT(SYSTEM_UPGRADE, "firmware upgrade schedule check started: [time=%02d:%02d]",
           firmwareManageCfg.scheduleTime.hour, firmwareManageCfg.scheduleTime.minute);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start firmware upgrade process
 * @param   upgradeType
 * @param   clientCbType
 * @param   clientSocket
 * @note    If one firmware check or download process already going on then other wan't allow.
 */
static NET_CMD_STATUS_e checkFirmwareUpdate(FirmwareUpgradeType_e upgradeType, CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket)
{
    FIRMWARE_MANAGEMENT_CONFIG_t firmwareManageCfg;

    /* Check process already running or not */
    MUTEX_LOCK(firmwareUpgradeInfo.threadLock);
    if (firmwareUpgradeInfo.isProcessRunning == TRUE)
    {
        /* Firmware upgrade process already running */
        MUTEX_UNLOCK(firmwareUpgradeInfo.threadLock);
        WPRINT(SYSTEM_UPGRADE, "firmware upgrade process already in progress: [upgradeType=%d]", firmwareUpgradeInfo.upgradeType);
        return CMD_REQUEST_IN_PROGRESS;
    }

    /* Firmware upgrade process started */
    firmwareUpgradeInfo.isProcessRunning = TRUE;
    MUTEX_UNLOCK(firmwareUpgradeInfo.threadLock);

    /* Stop firmware download retry timer if running */
    stopFirmwareDownloadTimer();

    /* Read config and start process */
    ReadFirmwareManagementConfig(&firmwareManageCfg);
    firmwareUpgradeInfo.upgradeType = upgradeType;
    firmwareUpgradeInfo.dirLevel = FTP_DIR_LEVEL_ONE;
    firmwareUpgradeInfo.clientCbType = clientCbType;
    firmwareUpgradeInfo.clientSocket = clientSocket;
    firmwareUpgradeInfo.firmwareResp = FIRMWARE_UPGRADE_RESP_OK;
    firmwareUpgradeInfo.ftpServerInfo = firmwareManageCfg.ftpServerInfo[firmwareManageCfg.ftpServerType];
    RESET_STR_BUFF(firmwareUpgradeInfo.firmwarePath);
    if (FAIL == Utils_CreateThread(NULL, checkFirmwareUpdateThread, NULL, DETACHED_THREAD, CHECK_UPDATE_THREAD_STACK_SZ))
    {
        /* Failed to start firmware update check thread */
        EPRINT(SYSTEM_UPGRADE, "fail to create firmware update check thread");
        resetFirmwareUpgradeInfo();
        return CMD_PROCESS_ERROR;
    }

    /* Firmware upgrade checking process started */
    DPRINT(SYSTEM_UPGRADE, "firmware upgrade check started: [upgradeType=%d]", firmwareUpgradeInfo.upgradeType);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Firmware update checking thread
 * @param   pThreadArg
 * @return
 */
static void *checkFirmwareUpdateThread(void *pThreadArg)
{
    FTP_RESPONSE_e      ftpResp;
    CHAR                replyMsg[MAX_RCV_SZ];
    UINT32              outLen;

    /* Set thread name */
    THREAD_START("FW_UPGRADE_CHK");

    do
    {
        /* Get FTP system type */
        ftpResp = GetFtpServerSystemType(&firmwareUpgradeInfo.ftpServerInfo, &firmwareUpgradeInfo.ftpSysType);
        if (ftpResp != FTP_SUCCESS)
        {
            /* Failed to get ftp system type */
            setFirmwareRespCode(ftpResp);
            EPRINT(SYSTEM_UPGRADE, "fail to get ftp system type: [resp=%s]", FIRMWARE_RESP_STR);
            break;
        }

        /* Get the FTP directory list for version-revision folder */
        firmwareUpgradeInfo.firmwareResp = FIRMWARE_UPGRADE_RESP_FIRMWARE_NOT_FOUND;
        ftpResp = ListFromFtp(&firmwareUpgradeInfo.ftpServerInfo, firmwareUpgradeInfo.ftpSysType, FALSE, NULL, NULL, ftpDirListCb);
        if (ftpResp != FTP_SUCCESS)
        {
            /* Failed to list FTP dir */
            setFirmwareRespCode(ftpResp);
            EPRINT(SYSTEM_UPGRADE, "fail to get ftp dir list: [resp=%s]", FIRMWARE_RESP_STR);
            break;
        }

    }while (0);

    /* Check firmware upgrade type */
    if (firmwareUpgradeInfo.upgradeType == FIRMWARE_UPGRADE_TYPE_AUTO_UPGRADE)
    {
        /* If success response found */
        if (firmwareUpgradeInfo.firmwareResp == FIRMWARE_UPGRADE_RESP_OK)
        {
            /* Start firmware download process timer */
            firmwareUpgradeInfo.firmwareDownloadRetry = 0;
            startFirmwareDownloadTimer(FIRMWARE_DOWNLOAD_INIT_TMR_IN_SEC);
        }
        else
        {
            /* Write event log for error response */
            writeEventLog(firmwareUpgradeInfo.firmwareResp);
        }
    }
    else if (firmwareUpgradeInfo.upgradeType == FIRMWARE_UPGRADE_TYPE_AUTO_NOTIFY)
    {
        /* If success response found */
        if (firmwareUpgradeInfo.firmwareResp == FIRMWARE_UPGRADE_RESP_OK)
        {
            /* Write event and notify event and action */
            WriteEvent(LOG_SYSTEM_EVENT, LOG_FIRMWARE_UPGRADE, NULL, firmwareUpgradeInfo.latestVersion, EVENT_UPGRADE);
            FirmwareUpgradeEvent(ACTIVE);
            startEventInactiveTimer();
        }
        else
        {
            /* Write event log for error response */
            writeEventLog(firmwareUpgradeInfo.firmwareResp);
        }
    }
    else
    {
        /* Client callback type and socket should be valid to send response */
        if ((firmwareUpgradeInfo.clientCbType < CLIENT_CB_TYPE_MAX) && (firmwareUpgradeInfo.clientSocket != INVALID_CONNECTION))
        {
            if (firmwareUpgradeInfo.firmwareResp == FIRMWARE_UPGRADE_RESP_OK)
            {
                /* Prepare response string */
                outLen = snprintf(replyMsg, sizeof(replyMsg), "%c%s%c%d%c%c%d%c%s%c%c%c",
                                  SOM, headerReq[RPL_CMD], FSP, CMD_SUCCESS, FSP, SOI, 1, FSP, firmwareUpgradeInfo.latestVersion, FSP, EOI, EOM);

                /* Send reply to client */
                sendCmdCb[firmwareUpgradeInfo.clientCbType](firmwareUpgradeInfo.clientSocket, (UINT8PTR)replyMsg, outLen, MESSAGE_REPLY_TIMEOUT);
                closeConnCb[firmwareUpgradeInfo.clientCbType](&firmwareUpgradeInfo.clientSocket);
                DPRINT(SYSTEM_UPGRADE, "new firmware available: [current=%s], [latest=%s]", GetSoftwareVersionStr(), firmwareUpgradeInfo.latestVersion);
            }
            else
            {
                /* Send error response to client */
                clientCmdRespCb[firmwareUpgradeInfo.clientCbType](respStatusCode[firmwareUpgradeInfo.firmwareResp], firmwareUpgradeInfo.clientSocket, TRUE);
                DPRINT(SYSTEM_UPGRADE, "firmware not available: [status=%s]", respStatusStr[firmwareUpgradeInfo.firmwareResp]);
            }
        }
    }

    /* Reset firmware upgrade info */
    resetFirmwareUpgradeInfo();

    /* Exit from thread */
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback function to list directories and files of FTP.
 * @param   dirList
 * @param   size
 * @param   userData
 * @return  SUCCESS / FAIL
 */
static BOOL ftpDirListCb(CHARPTR dirList, UINT32 size, VOIDPTR userData)
{
    CHARPTR         tmpListPtr = dirList;
    FTP_RESPONSE_e  ftpResp;
    CHAR            entry[FTP_REMOTE_PATH_LEN];
    BOOL            isDir;
    UINT16          outLen;

    /* Validate input params */
    if ((dirList == NULL) || (size == 0))
    {
        /* Invalid input params found */
        return SUCCESS;
    }

    /* Check level for recursive function call */
    if (firmwareUpgradeInfo.dirLevel == FTP_DIR_LEVEL_ONE)
    {
        INT32   argCnt;
        BOOL    isNewFirmwareFound = FALSE;
        CHAR    highestEntry[FTP_REMOTE_PATH_LEN];
        CHAR    fwVersionFolder[DEVICE_SW_VERSION_LEN_MAX];
        UINT16  version, highestVersion = SOFTWARE_VERSION;
        UINT16  revision, highestRevision = SOFTWARE_REVISION;
        UINT16  subRevision, highestSubRevision = PRODUCT_SUB_REVISION;

        /* Next time check next FTP dir level */
        firmwareUpgradeInfo.dirLevel++;

        /* Traverse whole list of directories */
        while ((tmpListPtr - dirList) < (INT32)size)
        {
            /* Parse till all entry are resolved */
            if (ParseFTPListReply(&tmpListPtr, entry, FTP_REMOTE_PATH_LEN, &isDir) == FAIL)
            {
                /* This is not a file or a folder */
                continue;
            }

            /* It should be directory only */
            if (isDir == FALSE)
            {
                /* This is a file, not a directory */
                continue;
            }

            /* It should have atleast 4 chars and should start with V */
            if ((strlen(entry) < VER_REV_STR_LEN_MIN) || (entry[0] != 'V'))
            {
                /* Directory doesn't have atleast 4 chars (V7R1) aor it doesn't start with V */
                continue;
            }

            /* Extract version, revision and sub revision */
            argCnt = sscanf(entry, FOLDER_VER_REV_SUB_REV_FORMAT_STR, &version, &revision, &subRevision);
            if (argCnt == 2)
            {
                /* Firmware doesn't have sub revision */
                snprintf(fwVersionFolder, sizeof(fwVersionFolder), FOLDER_VER_REV_FORMAT_STR"/", version, revision);
                subRevision = 0;
            }
            else if (argCnt == 3)
            {
                /* Firmware with sub revision */
                snprintf(fwVersionFolder, sizeof(fwVersionFolder), FOLDER_VER_REV_SUB_REV_FORMAT_STR"/", version, revision, subRevision);
            }
            else
            {
                /* Invalid folder found */
                continue;
            }

            /* Version and revision should be valid */
            if ((version == 0) || (revision == 0) || (strcmp(fwVersionFolder, entry)))
            {
                /* Invalid version and revision found */
                continue;
            }

            /* We have found firmware directory */
            firmwareUpgradeInfo.firmwareResp = FIRMWARE_UPGRADE_RESP_FIRMWARE_UP_TO_DATE;

            /* Firmware should be higher then existing system firmware */
            if ((version < highestVersion) || ((version == highestVersion) && (revision < highestRevision))
                    || ((version == highestVersion) && (revision == highestRevision) && (subRevision <= highestSubRevision)))
            {
                /* Lower or same version found */
                continue;
            }

            /* Found version is higher then current version */
            highestVersion = version;
            highestRevision = revision;
            highestSubRevision = subRevision;
            isNewFirmwareFound = TRUE;

            /* Store version related information for future use */
            DPRINT(SYSTEM_UPGRADE, "new firmware found: [version=%d], [revision=%d], [subRevision=%d]", version, revision, subRevision);
            snprintf(highestEntry, sizeof(highestEntry), "%s", entry);
            snprintf(firmwareUpgradeInfo.firmwarePath, sizeof(firmwareUpgradeInfo.firmwarePath),
                     "%sweb/"FIRMWARE_NAME_PREFIX"_"ZIP_FILE_VER_REV_STR, entry, version, revision);
            if (subRevision)
            {
                /* Store version-revision with sub version */
                snprintf(firmwareUpgradeInfo.latestVersion, sizeof(firmwareUpgradeInfo.latestVersion),
                         FIRMWARE_VER_REV_SUB_REV_FORMAT_STR, version, revision, subRevision);
            }
            else
            {
                /* Store version-revision without sub version */
                snprintf(firmwareUpgradeInfo.latestVersion, sizeof(firmwareUpgradeInfo.latestVersion),
                         FIRMWARE_VER_REV_FORMAT_STR, version, revision);
            }
        }

        /* New firmware folder found */
        if (TRUE == isNewFirmwareFound)
        {
            /* Get the FTP directory list for web folder */
            firmwareUpgradeInfo.firmwareResp = FIRMWARE_UPGRADE_RESP_FIRMWARE_NOT_FOUND;
            ftpResp = ListFromFtp(&firmwareUpgradeInfo.ftpServerInfo, firmwareUpgradeInfo.ftpSysType, FALSE, highestEntry, (VOIDPTR)highestEntry, ftpDirListCb);
            if (ftpResp != FTP_SUCCESS)
            {
                /* Failed to list FTP dir */
                setFirmwareRespCode(ftpResp);
                EPRINT(SYSTEM_UPGRADE, "fail to get level 2 list: [version=%s], [resp=%s]", firmwareUpgradeInfo.latestVersion, FIRMWARE_RESP_STR);
            }
        }

        /* Set FTP dir level down by one */
        firmwareUpgradeInfo.dirLevel--;
    }
    else if (firmwareUpgradeInfo.dirLevel == FTP_DIR_LEVEL_TWO)
    {
        /* Next time check next FTP dir level */
        firmwareUpgradeInfo.dirLevel++;

        /* Copy previous directory path and length */
        snprintf(entry, FTP_REMOTE_PATH_LEN, "%s", (CHARPTR)userData);
        outLen = strlen(entry);

        /* Traverse whole list of directories */
        while ((tmpListPtr - dirList) < (INT32)size)
        {
            /* Parse till all entry are resolved */
            if (ParseFTPListReply(&tmpListPtr, (entry + outLen), FTP_REMOTE_PATH_LEN, &isDir) == FAIL)
            {
                /* This is not a file or a folder */
                continue;
            }

            /* It should be directory only */
            if (isDir == FALSE)
            {
                /* This is a file, not a directory */
                continue;
            }

            /* We are looking for web folder */
            if (strcmp((entry + outLen), "web/"))
            {
                /* Web folder not found */
                continue;
            }

            /* Get the FTP directory list for firmware file */
            ftpResp = ListFromFtp(&firmwareUpgradeInfo.ftpServerInfo, firmwareUpgradeInfo.ftpSysType, FALSE, entry, (VOIDPTR)entry, ftpDirListCb);
            if (ftpResp != FTP_SUCCESS)
            {
                /* Failed to list FTP dir */
                setFirmwareRespCode(ftpResp);
                EPRINT(SYSTEM_UPGRADE, "fail to get level 3 list: [version=%s], [resp=%s]", firmwareUpgradeInfo.latestVersion, FIRMWARE_RESP_STR);
            }
            break;
        }

        /* Set FTP dir level down by one */
        firmwareUpgradeInfo.dirLevel--;
    }
    else if (firmwareUpgradeInfo.dirLevel == FTP_DIR_LEVEL_THREE)
    {
        /* Next time check next FTP dir level */
        firmwareUpgradeInfo.dirLevel++;

        /* Copy previous directory path and length */
        snprintf(entry, FTP_REMOTE_PATH_LEN, "%s", (CHARPTR)userData);
        outLen = strlen(entry);

        /* Traverse whole list of directories */
        while ((tmpListPtr - dirList) < (INT32)size)
        {
            /* Parse till all entry are resolved */
            if (ParseFTPListReply(&tmpListPtr, (entry + outLen), FTP_REMOTE_PATH_LEN, &isDir) == FAIL)
            {
                /* This is not a file or a folder */
                continue;
            }

            /* It should not be directory */
            if (isDir == TRUE)
            {
                /* This is not a file */
                continue;
            }

            /* Firmware path should be completely matched with our prepared path */
            if (strcmp(firmwareUpgradeInfo.firmwarePath, entry) == 0)
            {
                /* Firmware file found for upgradation */
                firmwareUpgradeInfo.firmwareResp = FIRMWARE_UPGRADE_RESP_OK;
                DPRINT(SYSTEM_UPGRADE, "firmware found for upgradation: [version=%s]", firmwareUpgradeInfo.latestVersion);
                break;
            }
        }

        /* Set FTP dir level down by one */
        firmwareUpgradeInfo.dirLevel--;
    }

    /* Dummy */
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Download and update system firmware
 * @param   upgradeType
 * @param   clientCbType
 * @param   clientSocket
 * @return
 */
static NET_CMD_STATUS_e downloadAndUpgradeFirmware(CLIENT_CB_TYPE_e clientCbType, INT32 clientSocket)
{
    /* Firmware download path should be valid. It indicates that we have already checked firmware upgrade process */
    if (firmwareUpgradeInfo.firmwarePath[0] == '\0')
    {
        EPRINT(SYSTEM_UPGRADE, "firmware path not found: [upgradeType=%d]", firmwareUpgradeInfo.upgradeType);
        return CMD_FIRMWARE_NOT_FOUND;
    }

    /* Check process already running or not */
    MUTEX_LOCK(firmwareUpgradeInfo.threadLock);
    if (firmwareUpgradeInfo.isProcessRunning == TRUE)
    {
        /* Firmware upgrade process already running */
        MUTEX_UNLOCK(firmwareUpgradeInfo.threadLock);
        WPRINT(SYSTEM_UPGRADE, "firmware upgrade process already in progress: [upgradeType=%d]", firmwareUpgradeInfo.upgradeType);
        return CMD_REQUEST_IN_PROGRESS;
    }

    /* Firmware upgrade process started */
    firmwareUpgradeInfo.isProcessRunning = TRUE;
    MUTEX_UNLOCK(firmwareUpgradeInfo.threadLock);

    /* Start firmware download process */
    firmwareUpgradeInfo.clientCbType = clientCbType;
    firmwareUpgradeInfo.clientSocket = clientSocket;
    firmwareUpgradeInfo.firmwareResp = FIRMWARE_UPGRADE_RESP_OK;
    if (FAIL == Utils_CreateThread(NULL, firmwareDownloadThread, NULL, DETACHED_THREAD, FIRMWARE_DOWNLOAD_THREAD_STACK_SZ))
    {
        /* Failed to start firmware download process */
        EPRINT(SYSTEM_UPGRADE, "fail to create firmware download thread");
        resetFirmwareUpgradeInfo();
        return CMD_PROCESS_ERROR;
    }

    /* Firmware upgrade checking process started */
    DPRINT(SYSTEM_UPGRADE, "firmware download started: [upgradeType=%d]", firmwareUpgradeInfo.upgradeType);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Firmware download thread
 * @param   pThreadArg
 * @return
 */
static void *firmwareDownloadThread(void *pThreadArg)
{
    FTP_FILE_INFO_t     ftpFileInfo;
    NET_CMD_STATUS_e    cmdResp;

    /* Set thread name */
    THREAD_START("FW_DOWNLOAD");

    /* Remove previous firmware file if present */
    unlink(FIRMWARE_FILE);

    /* Copy remote and local file path to be download */
    snprintf(ftpFileInfo.remoteFile, sizeof(ftpFileInfo.remoteFile), "%s", firmwareUpgradeInfo.firmwarePath);
    snprintf(ftpFileInfo.localFileName, sizeof(ftpFileInfo.localFileName), FIRMWARE_FILE);

    /* Start firmware download process from FTP */
    cmdResp = StartFirmwareDownload(&firmwareUpgradeInfo.ftpServerInfo, &ftpFileInfo, ftpFirmwareDownloadCb, firmwareUpgradeInfo.upgradeType);

    /* Client callback type and socket should be valid to send response */
    if ((firmwareUpgradeInfo.clientCbType < CLIENT_CB_TYPE_MAX) && (firmwareUpgradeInfo.clientSocket != INVALID_CONNECTION))
    {
        /* Send response to the client */
        clientCmdRespCb[firmwareUpgradeInfo.clientCbType](cmdResp, firmwareUpgradeInfo.clientSocket, TRUE);
    }

    /* If firmware download start failed then retry after some time */
    if (cmdResp != CMD_SUCCESS)
    {
        /* Retry for firmware download if type auto download and upgrade */
        if (firmwareUpgradeInfo.upgradeType == FIRMWARE_UPGRADE_TYPE_AUTO_UPGRADE)
        {
            if (TRUE == isFirmwareDownloadRetryDone(FTP_FAIL))
            {
                /* Failed to download firmware after multiple retries */
                EPRINT(SYSTEM_UPGRADE, "fail to download firmware after multiple retries: [status=%s]", FIRMWARE_RESP_STR);
            }
        }

        /* Reset firmware upgrade info */
        resetFirmwareUpgradeInfo();
        EPRINT(SYSTEM_UPGRADE, "fail to start firmware download process: [cmdResp=%d], [upgradeType=%d]", cmdResp, firmwareUpgradeInfo.upgradeType);
    }

    /* Exit from thread */
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ftpFirmwareDownloadCb
 * @param   ftpHandle
 * @param   ftpResp
 * @param   userData
 */
static void ftpFirmwareDownloadCb(FTP_HANDLE ftpHandle, FTP_RESPONSE_e ftpResp, UINT16 userData)
{
    /* Check FTP response status */
    if (ftpResp != FTP_SUCCESS)
    {
        /* If firmware download for auto upgrade then retry on failure */
        if (userData == FIRMWARE_UPGRADE_TYPE_AUTO_UPGRADE)
        {
            /* Retry for firmware download */
            if (TRUE == isFirmwareDownloadRetryDone(ftpResp))
            {
                /* Failed to download firmware after multiple retries */
                EPRINT(SYSTEM_UPGRADE, "fail to download firmware after multiple retries: [ftpResp=%d]", ftpResp);
            }
        }
        else
        {
            /* For manual case write just event log */
            writeEventLog(FIRMWARE_UPGRADE_RESP_UNKNOWN);
        }

        /* Reset firmware upgrade info */
        resetFirmwareUpgradeInfo();
        EPRINT(SYSTEM_UPGRADE, "firmware download fail: [ftpResp=%d], [upgradeType=%d]", ftpResp, userData);
        return;
    }

    /* Reset firmware upgrade info */
    resetFirmwareUpgradeInfo();

    /* Check file available or not */
    if (access(FIRMWARE_FILE, F_OK) != STATUS_OK)
    {
        /* File not present in file system */
        EPRINT(SYSTEM_UPGRADE, "firmware download success but file not present");
        return;
    }

    /* Check file size */
    struct stat stateInfo = {0};
    if (stat(FIRMWARE_FILE, &stateInfo) != STATUS_OK)
    {
        /* Failed to get file size */
        EPRINT(SYSTEM_UPGRADE, "fail to stat downloaded firmware file: [err=%s]", STR_ERR);
        return;
    }

    /* Firmware should have atleast some size */
    if (stateInfo.st_size < MEGA_BYTE)
    {
        /* File size is too small */
        EPRINT(SYSTEM_UPGRADE, "invld downloaded firmware file size: [size=%lld]", (UINT64)stateInfo.st_size);
        return;
    }

    /* Stop timer if running and reboot the system for firmware upgrade */
    stopFirmwareDownloadTimer();
    DPRINT(SYSTEM_UPGRADE, "firmware download success, restart system for upgrade");
    PrepForPowerAction(REBOOT_DEVICE, EVENT_AUTO, "UPGRADE");
    WriteEvent(LOG_OTHER_EVENT, LOG_UPGRADE_RESULT, NULL, NULL, EVENT_SUCCESS);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start firmware download retry timer
 * @param   timeInSec
 */
static void startFirmwareDownloadTimer(UINT16 timeInSec)
{
    TIMER_INFO_t timerInfo;

    /* If timer handle is valid then reload timer else start new timer */
    MUTEX_LOCK(firmwareUpgradeInfo.timerLock);
    if (firmwareUpgradeInfo.timerHandle == INVALID_TIMER_HANDLE)
    {
        /* Start new timer */
        timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(timeInSec);
        timerInfo.data = 0;
        timerInfo.funcPtr = firmwareDownloadTimerCb;
        StartTimer(timerInfo, &firmwareUpgradeInfo.timerHandle);
    }
    else
    {
        /* Reload existing timer */
        ReloadTimer(firmwareUpgradeInfo.timerHandle, CONVERT_SEC_TO_TIMER_COUNT(timeInSec));
    }
    MUTEX_UNLOCK(firmwareUpgradeInfo.timerLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Stop firmware download retry timer
 */
static void stopFirmwareDownloadTimer(void)
{
    /* Delete timer if running */
    MUTEX_LOCK(firmwareUpgradeInfo.timerLock);
    if (firmwareUpgradeInfo.timerHandle != INVALID_TIMER_HANDLE)
    {
        DeleteTimer(&firmwareUpgradeInfo.timerHandle);
    }
    MUTEX_UNLOCK(firmwareUpgradeInfo.timerLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Firmware download timer callback
 * @param   data
 */
static void firmwareDownloadTimerCb(UINT32 data)
{
    /* Start firmware download process as timer expire */
    if (CMD_SUCCESS == downloadAndUpgradeFirmware(CLIENT_CB_TYPE_MAX, INVALID_CONNECTION))
    {
        /* Stop timer as process started successfully */
        stopFirmwareDownloadTimer();
        return;
    }

    /* Retry for firmware download */
    if (TRUE == isFirmwareDownloadRetryDone(FTP_FAIL))
    {
        /* Failed to download firmware after multiple retries */
        EPRINT(SYSTEM_UPGRADE, "fail to download firmware after multiple retries");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check whether all retry done or not. If not, then start timer and download firmware
 *          else stop timer and add reason in event log.
 * @param   ftpResp
 * @return  TRUE / FALSE
 */
static BOOL isFirmwareDownloadRetryDone(FTP_RESPONSE_e ftpResp)
{
    /* Check for firmware download retry */
    firmwareUpgradeInfo.firmwareDownloadRetry++;
    if (firmwareUpgradeInfo.firmwareDownloadRetry >= FIRMWARE_DOWNLOAD_RETRY_MAX)
    {
        /* We have tried for max but couldn't download the firmware */
        stopFirmwareDownloadTimer();

        /* Write event log on failure */
        setFirmwareRespCode(ftpResp);
        writeEventLog(firmwareUpgradeInfo.firmwareResp);
        return TRUE;
    }
    else
    {
        /* Better luck next time */
        startFirmwareDownloadTimer(FIRMWARE_DOWNLOAD_RETRY_TMR_IN_SEC);
        WPRINT(SYSTEM_UPGRADE, "firmware download retry timer started: [retryCnt=%d]", firmwareUpgradeInfo.firmwareDownloadRetry);
        return FALSE;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start event inactive timer
 */
static void startEventInactiveTimer(void)
{
    TIMER_INFO_t timerInfo;

    /* If timer handle is valid then reload timer else start new timer */
    MUTEX_LOCK(firmwareUpgradeInfo.timerLock);
    if (firmwareUpgradeInfo.eventTimerHandle == INVALID_TIMER_HANDLE)
    {
        /* Start new timer */
        timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(5);
        timerInfo.data = 0;
        timerInfo.funcPtr = eventInactiveTimerCb;
        StartTimer(timerInfo, &firmwareUpgradeInfo.eventTimerHandle);
    }
    else
    {
        /* Reload existing timer */
        ReloadTimer(firmwareUpgradeInfo.eventTimerHandle, CONVERT_SEC_TO_TIMER_COUNT(5));
    }
    MUTEX_UNLOCK(firmwareUpgradeInfo.timerLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Make event inactive timer callback
 * @param   data
 */
static void eventInactiveTimerCb(UINT32 data)
{
    /* Make firmware upgrade event inactive */
    FirmwareUpgradeEvent(INACTIVE);

    /* Delete timer if running */
    MUTEX_LOCK(firmwareUpgradeInfo.timerLock);
    if (firmwareUpgradeInfo.eventTimerHandle != INVALID_TIMER_HANDLE)
    {
        DeleteTimer(&firmwareUpgradeInfo.eventTimerHandle);
    }
    MUTEX_UNLOCK(firmwareUpgradeInfo.timerLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write event log on error
 * @param   firmwareResp
 */
static void writeEventLog(FirmwareUpgradeResp_e firmwareResp)
{
    /* Log event for specific errors */
    if (firmwareResp == FIRMWARE_UPGRADE_RESP_FIRMWARE_NOT_FOUND)
    {
        WriteEvent(LOG_OTHER_EVENT, LOG_UPGRADE_RESULT, NULL, "Firmware not found", EVENT_FAIL);
    }
    else if (firmwareResp == FIRMWARE_UPGRADE_RESP_CONNECT_ERROR)
    {
        WriteEvent(LOG_OTHER_EVENT, LOG_UPGRADE_RESULT, NULL, "Error while connecting", EVENT_FAIL);
    }
    else if (firmwareResp == FIRMWARE_UPGRADE_RESP_UNKNOWN)
    {
        WriteEvent(LOG_OTHER_EVENT, LOG_UPGRADE_RESULT, NULL, "Downloading Error", EVENT_FAIL);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set firmware response code
 * @param   ftpResp
 */
static void setFirmwareRespCode(FTP_RESPONSE_e ftpResp)
{
    /* Map FTP response code with firmware upgrade response code */
    if (ftpResp == FTP_CONNECTION_ERROR)
    {
        firmwareUpgradeInfo.firmwareResp = FIRMWARE_UPGRADE_RESP_CONNECT_ERROR;
    }
    else if (ftpResp == FTP_AUTH_ERROR)
    {
        firmwareUpgradeInfo.firmwareResp = FIRMWARE_UPGRADE_RESP_AUTH_ERROR;
    }
    else
    {
        firmwareUpgradeInfo.firmwareResp = FIRMWARE_UPGRADE_RESP_UNKNOWN;
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
