//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		Utils.c
@brief      File containing the definations of different utils functions for all modules
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <signal.h>
#include <netdb.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>

/* Library Includes */
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/sha.h>

/* Application Includes */
#include "Utils.h"
#include "DebugLog.h"
#include "SysTimer.h"
#include "MxMp2TsParser.h"
#include "NetworkManager.h"
#include "InputOutput.h"
#include "NetworkInterface.h"
#if defined(RK3568_NVRL)
#include "watchdog.h"
#endif

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Path of system command executor application */
#define SYS_CMD_EXE_APPL_PATH       BIN_DIR_PATH "/sysCmdExe.bin"

/* Format: HH:MM time in minutes */
#define TIME_IN_MIN(hr, min)		((hr * SEC_IN_ONE_MIN) + min)

#define DOMAIN_NAME_STR				"domain"
#define PROC_PATH					"/proc/"

#define FTP_CONFIG_FILE             "# Example config file /etc/vsftpd.conf\n"      \
                                    "anonymous_enable=NO\n"                         \
                                    "local_enable=YES\n"                            \
                                    "write_enable=NO\n"                             \
                                    "dirmessage_enable=YES\n"                       \
                                    "xferlog_enable=YES\n"                          \
                                    "connect_from_port_20=YES\n"                    \
                                    "ftpd_banner=Welcome Satatya NVR "              \
                                    "FTP service.\n"                                \
                                    "chroot_list_enable=YES\n"                      \
                                    "chroot_list_file=/etc/vsftpd.chroot_list\n"    \
                                    "local_root=" MEDIA_DIR_PATH "\n"               \
                                    "listen=YES\n"                                  \
                                    "allow_writeable_chroot=YES\n"                  \
                                    "listen_port="

#define FTP_FILE_NAME				"/etc/vsftpd.conf"
#define FTP_START_CMD				"start-stop-daemon -S -b -x vsftpd"
#define FTP_STOP_CMD				"start-stop-daemon -K -x vsftpd"
#define NO_OF_SPACE_TO_GET_NAME     8
#define WEBSERVER_START_CMD         "start-stop-daemon -S -b -x webserver -- -p %d"
#define WEBSERVER_STOP_CMD          "start-stop-daemon -K -x webserver"

/* Max chunk size for file transfer from one location to other */
#define FILE_RD_WR_CHUNK_SIZE_MAX   (MEGA_BYTE)

// Usage: time to Wait for LOG_SHUTDOWN live event to be sent to all client
#define SYS_SHUTDOWN_WAIT_TIME 		5 	// seconds
#define MAX_RTP_PORT_COUNT			8192

#define SYS_MONITOR_TMR_IN_SEC      1
#define CPU_STATUS_CALC_TIME        3	// Seconds
#define CPU_MAX_TEMP_FOR_BUZZER		(100)
#define CPU_MAX_TEMP_FOR_SHUTDOWN	(105)
#define CPU_TEMP_DEBOUNCE_COUNT		(3)
#define CPU_TEMP_ADV_DETAIL			"Abnormal CPU Temperature"

#define PROC_STAT_FILE 				"/proc/stat"
#define PROC_NET_DEV_FILE			"/proc/net/dev"
#define PROC_UPTIME_FILE 			"/proc/uptime"

#define ALPHA_NUM_STR_LEN_MAX       63
#define CHAR64(c)                   (((c) < 0 || (c) > 127) ? -1 : index_64[(c)])

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
	UINT64 usr, nic, sys, idle;
	UINT64 iowait, irq, softirq, steal;
	UINT64 total;
	UINT64 busy;
}JIFFY_COUNTS_t;

typedef struct
{
    UINT64  prevBusyCnt;
    UINT64  prevTotalCnt;
    UINT8   busyPercent;
}CPU_LOAD_PARAM_t;

typedef struct
{
    UINT64  txCnt;
    UINT64  rxCnt;
}NET_INTERFACE_INFO_PARAM_t;

typedef struct
{
	CPU_LOAD_PARAM_t			cpuLoad;
    NET_INTERFACE_INFO_PARAM_t	prevInfo[NETWORK_PORT_MAX];
    NET_INTERFACE_LOAD_PARAM_t	ifLoad[NETWORK_PORT_MAX];
}STATUS_PARAM_t;

typedef struct
{
	UINT8				rtpPortNo[MAX_RTP_PORT_COUNT];		// Take all port as bits
	RTP_PORT_RANGE_t	rtpPort;
	pthread_mutex_t		rtpPortMutex;
}RTP_PORT_SETTING_t;

typedef struct
{
    BOOL            isExeStarted;
    INT32           txMsgQId;
    INT32           rxMsgQId;
    UINT16          cmdId;
    pthread_mutex_t cmdIdMutex;
}SYS_CMD_IPC_INFO_t;

typedef struct
{
    POWER_ACTION_e  action;
    TIMER_HANDLE    timer;
    pthread_mutex_t mutex;

}POWER_ACTION_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const CHARPTR        monthName[MAX_MONTH] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static const CHARPTR        weekName[MAX_MONTH] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const CHARPTR        powerActStr[POWER_ACTION_MAX] = {"SHUTDOWN", "RESTART"};
static TIMER_HANDLE 		systemMonitorTimerHandle = INVALID_TIMER_HANDLE;
static RTP_PORT_SETTING_t   rtpPortInfo = {{0}, {0,0}, PTHREAD_MUTEX_INITIALIZER};
static INT32 				watchdogFd = INVALID_FILE_FD;
static STATUS_PARAM_t		statusParam;

static POWER_ACTION_INFO_t  powerActionInfo = {POWER_ACTION_MAX, INVALID_TIMER_HANDLE, PTHREAD_MUTEX_INITIALIZER};
static SYS_CMD_IPC_INFO_t   sysCmdIpcInfo = {FALSE, INVALID_FILE_FD, INVALID_FILE_FD, 0, PTHREAD_MUTEX_INITIALIZER};
static const CHAR           index_64[128] =
{
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
    52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
    -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
    -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
    41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL communicateWithSysCmdExecutor(const CHARPTR pSysCmd);
//-------------------------------------------------------------------------------------------------
static void systemStatusMonitorTmrCb(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void updateCpuStatusParam(void);
//-------------------------------------------------------------------------------------------------
static void sysPowerActionTimerCb(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL readCpuJiffy(JIFFY_COUNTS_t *pJiffy);
//-------------------------------------------------------------------------------------------------
static BOOL getAllNetworkInterfaceInfo(NET_INTERFACE_INFO_PARAM_t *ifInfo);
//-------------------------------------------------------------------------------------------------
static void uninRowColumnBoxes(UINT8PTR unitBoxColumn ,UINT8PTR unitBoxstartColumn, UINT8PTR unitBoxRow, UINT8PTR unitBoxstartRow, UINT8 row, UINT8 column);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initialise utils module.
 */
void InitUtils(void)
{
    GENERAL_CONFIG_t	genCfg;
    TIMER_INFO_t        timerInfo;
    JIFFY_COUNTS_t		cpuLoad = { 0 };

    ReadGeneralConfig(&genCfg);
    MUTEX_INIT(rtpPortInfo.rtpPortMutex, NULL);
	rtpPortInfo.rtpPort = genCfg.rtpPort;
    memset(rtpPortInfo.rtpPortNo, 0, sizeof(rtpPortInfo.rtpPortNo));

    /* Initialize mp2Ts client */
	InitMp2TsClient();

    /* Start system status monitor timer */
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(SYS_MONITOR_TMR_IN_SEC);
    timerInfo.data = 0;
    timerInfo.funcPtr = systemStatusMonitorTmrCb;
    StartTimer(timerInfo, &systemMonitorTimerHandle);

    /* Reset and get network statistics */
    memset(statusParam.ifLoad, 0, sizeof(statusParam.ifLoad));
	getAllNetworkInterfaceInfo(statusParam.prevInfo);

    readCpuJiffy(&cpuLoad);
    statusParam.cpuLoad.prevBusyCnt = cpuLoad.busy;
    statusParam.cpuLoad.prevTotalCnt = cpuLoad.total;
	statusParam.cpuLoad.busyPercent = 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initialization of System command handler
 * @return  SUCCESS/FAIL
 */
BOOL InitSysCmdHandler(void)
{
    /* If queue is already exist then first delete that */
    sysCmdIpcInfo.txMsgQId = msgget(SYS_CMD_EXE_REQ_IPC_MSG_ID, 0666);
    if (sysCmdIpcInfo.txMsgQId != INVALID_FILE_FD)
    {
        /* Queue is present, delete it */
        if (msgctl(sysCmdIpcInfo.txMsgQId, IPC_RMID, 0) == -1)
        {
            /* Failed to delete the queue */
            EPRINT(UTILS, "fail to delete system cmd tx msg queue id: [err=%s]", STR_ERR);
        }
    }

    /* Create the message queue for system command execution */
    sysCmdIpcInfo.txMsgQId = msgget(SYS_CMD_EXE_REQ_IPC_MSG_ID, IPC_CREAT | IPC_EXCL | 0666);
    if (sysCmdIpcInfo.txMsgQId == INVALID_FILE_FD)
    {
        /* Fail to create system command ipc message queue */
        EPRINT(UTILS, "fail to get system cmd tx msg queue id: [err=%s]", STR_ERR);
        return FAIL;
    }

    /* If queue is already exist then first delete that */
    sysCmdIpcInfo.rxMsgQId = msgget(SYS_CMD_EXE_RESP_IPC_MSG_ID, 0666);
    if (sysCmdIpcInfo.rxMsgQId != INVALID_FILE_FD)
    {
        /* Queue is present, delete it */
        if (msgctl(sysCmdIpcInfo.rxMsgQId, IPC_RMID, 0) == -1)
        {
            /* Failed to delete the queue */
            EPRINT(UTILS, "fail to delete system cmd rx msg queue id: [err=%s]", STR_ERR);
        }
    }

    /* Create the message queue for system command execution */
    sysCmdIpcInfo.rxMsgQId = msgget(SYS_CMD_EXE_RESP_IPC_MSG_ID, IPC_CREAT | IPC_EXCL | 0666);
    if (sysCmdIpcInfo.rxMsgQId == INVALID_FILE_FD)
    {
        /* Fail to create system command ipc message queue */
        EPRINT(UTILS, "fail to get system cmd rx msg queue id: [err=%s]", STR_ERR);
        return FAIL;
    }

    /* System command exe ipc message queue created successfully */
    DPRINT(UTILS, "system cmd msg queue created successfully");

    /* Start system command executor application */
    if (FAIL == ExeSysCmd(FALSE, SYS_CMD_EXE_APPL_PATH " &"))
    {
        /* Fail to start the system command executor application */
        EPRINT(UTILS, "fail to start system cmd exe app");
        return FAIL;
    }

    /* System command executor started successfully */
    DPRINT(UTILS, "system cmd exe started successfully");
    sysCmdIpcInfo.isExeStarted = TRUE;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Execute system command
 * @param   useSysCmdExeF
 * @param   rSysCmd
 * @return  SUCCESS/FAIL
 */
BOOL ExeSysCmd(BOOL useSysCmdExeF, const CHARPTR rSysCmd)
{
    /* If queue is not created then execute command directly */
    if ((FALSE == useSysCmdExeF) || (FALSE == sysCmdIpcInfo.isExeStarted) || (strlen(rSysCmd) >= SYS_CMD_MSG_LEN_MAX))
    {
        /* Execute system command using popen */
        return Utils_ExeCmd(rSysCmd);
    }

    /* Send system command to executor */
    if (FAIL == communicateWithSysCmdExecutor(rSysCmd))
    {
        EPRINT(UTILS, "fail to execute cmd: [cmd=%s]", rSysCmd);
        return FAIL;
    }

    /* Command executed successfully */
    DPRINT(UTILS, "cmd executed: [cmd=%s]", rSysCmd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Execute sensitive information system command
 * @param   rSysCmd
 * @return  SUCCESS/FAIL
 */
BOOL ExeSensitiveInfoSysCmd(const CHARPTR rSysCmd)
{
    /* Send system command to executor */
    return communicateWithSysCmdExecutor(rSysCmd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Execute system command from system command executor binary
 * @param   pSysCmd
 * @return  SUCCESS/FAIL
 */
static BOOL communicateWithSysCmdExecutor(const CHARPTR pSysCmd)
{
    UINT16              sysCmdId;
    SYS_CMD_MSG_INFO_t  sysCmdMsg;

    /* Get the uniq command id */
    MUTEX_LOCK(sysCmdIpcInfo.cmdIdMutex);
    sysCmdIpcInfo.cmdId++;
    if (sysCmdIpcInfo.cmdId < SYS_EXE_CTRL_CMD_ID_MAX)
    {
        /* Don't use special reserve command Ids. Few are reserved to control command executor.
         * Zero is used for any msgtype in ipc queue. So we can't use it due to our design */
        sysCmdIpcInfo.cmdId = SYS_EXE_CTRL_CMD_ID_MAX;
    }
    sysCmdId = sysCmdIpcInfo.cmdId;
    MUTEX_UNLOCK(sysCmdIpcInfo.cmdIdMutex);

    /* Init ipc msg info */
    sysCmdMsg.cmdId = sysCmdId;
    snprintf(sysCmdMsg.cmdData, SYS_CMD_MSG_LEN_MAX, "%s", pSysCmd);

    /* Send system command message to executor */
    if ((msgsnd(sysCmdIpcInfo.txMsgQId, (void*)&sysCmdMsg, strlen(sysCmdMsg.cmdData)+1, IPC_NOWAIT)) == -1)
    {
        EPRINT(UTILS, "fail to send msg to sys cmd handler: [sysCmdId=%d], [err=%s]", sysCmdId, STR_ERR);
        return Utils_ExeCmd(pSysCmd);
    }

    /* Receive response from executor */
    if ((msgrcv(sysCmdIpcInfo.rxMsgQId, (void*)&sysCmdMsg, SYS_CMD_MSG_LEN_MAX, sysCmdId, 0)) == -1)
    {
        EPRINT(UTILS, "fail to recv resp from sys cmd handler: [sysCmdId=%d], [err=%s]", sysCmdId, STR_ERR);
        return FAIL;
    }

    /* Validate command type */
    if (sysCmdMsg.cmdId != sysCmdId)
    {
        EPRINT(UTILS, "invld cmd type recv: [txCmdId=%d], [rxCmdId=%ld]", sysCmdId, sysCmdMsg.cmdId);
        return FAIL;
    }

    /* Validate response */
    if (FAIL == sysCmdMsg.cmdData[0])
    {
        return FAIL;
    }

    /* Command executed successfully */
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Debug config change notify to system command executor
 */
void SetCmdExeDebugConfig(void)
{
    SYS_EXE_CTRL_CMD_ID_e   ctrlCmdId = SYS_EXE_CTRL_CMD_ID_DEBUG_CNFG;
    SYS_CMD_MSG_INFO_t      sysCmdMsg;

    /* Nothing to do if command executor is not running */
    if (FALSE == sysCmdIpcInfo.isExeStarted)
    {
        return;
    }

    /* Init ipc msg info */
    sysCmdMsg.cmdId = ctrlCmdId;

    /* Send debug config change command message to executor */
    if ((msgsnd(sysCmdIpcInfo.txMsgQId, (void*)&sysCmdMsg, 0, IPC_NOWAIT)) == -1)
    {
        EPRINT(UTILS, "fail to send debug config change msg: [err=%s]", STR_ERR);
        return;
    }

    /* Message sent successfully to executor */
    DPRINT(SYS_LOG, "debug config change msg sent");

    /* Receive response from executor */
    if ((msgrcv(sysCmdIpcInfo.rxMsgQId, (void*)&sysCmdMsg, SYS_CMD_MSG_LEN_MAX, ctrlCmdId, 0)) == -1)
    {
        EPRINT(UTILS, "fail to recv resp of debug config change msg: [err=%s]", STR_ERR);
        return;
    }

    /* Validate command type */
    if (sysCmdMsg.cmdId != (long)ctrlCmdId)
    {
        EPRINT(UTILS, "invld cmd type recv for debug config change: [txCmdId=%d], [rxCmdId=%ld]", ctrlCmdId, sysCmdMsg.cmdId);
        return;
    }

    /* Validate response */
    if (FAIL == sysCmdMsg.cmdData[0])
    {
        EPRINT(UTILS, "fail to update debug config");
        return;
    }

    /* Debug config updated successfully by system command executor */
    DPRINT(UTILS, "debug config updated successfully");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will find whether the given cmp time is within the given start time and
 *          end time or not. Time is in HH/MM format.
 * @param   cmpTime
 * @param   startTime
 * @param   endTime
 * @return  It returns TRUE if yes otherwise it returns FALSE.
 */
BOOL IsGivenTimeInWindow(TIME_HH_MM_t cmpTime, TIME_HH_MM_t startTime, TIME_HH_MM_t endTime)
{
	// Convert time into minute counts since midnight
    UINT16 startTimeInMins = TIME_IN_MIN(startTime.hour, startTime.minute);
    UINT16 endTimeInMins = TIME_IN_MIN(endTime.hour, endTime.minute);

	// changes to support 23:55 to 00:00 time interval as per change log 44 of v3r4
    if((endTimeInMins == 0) && (startTimeInMins > 0))
	{
		endTimeInMins = 1440; //23*60 + 59m + 59s
	}

	// Is start time and end time is of same day
    if(endTimeInMins > startTimeInMins)
	{
        UINT16 cmpTimeInMins = TIME_IN_MIN(cmpTime.hour, cmpTime.minute);
        if((cmpTimeInMins >= startTimeInMins) && (cmpTimeInMins < endTimeInMins))
		{
            return TRUE;
		}
	}

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function send signal to main loop for poweroff device or reboot device.
 * @param   action
 */
void SetPowerAction(POWER_ACTION_e action)
{
    powerActionInfo.action = action;
    DPRINT(SYS_LOG, "set power action: [action=%s]", powerActStr[action]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It gives current power action.
 * @return  Power action
 */
POWER_ACTION_e GetPowerAction(void)
{
    return powerActionInfo.action;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is configuration update callback. When any notification changed its call by other module.
 * @param   newGeneralConfig
 * @param   oldGeneralConfig
 */
void UtilsGeneralCfgUpdate(GENERAL_CONFIG_t newGeneralConfig, GENERAL_CONFIG_t *oldGeneralConfig)
{
    const CHARPTR pRecordFormatEventStr[MAX_REC_FORMAT] =
    {
        "Record Type Changed to Native",
        "Record Type Changed to Both",
        "Record Type Changed to Avi"
    };

    if(oldGeneralConfig->rtpPort.start != newGeneralConfig.rtpPort.start)
    {
        MUTEX_LOCK(rtpPortInfo.rtpPortMutex);
        rtpPortInfo.rtpPort = newGeneralConfig.rtpPort;
        MUTEX_UNLOCK(rtpPortInfo.rtpPortMutex);
    }

    if (oldGeneralConfig->recordFormatType != newGeneralConfig.recordFormatType)
    {
        if (newGeneralConfig.recordFormatType < MAX_REC_FORMAT)
        {
            PrepForPowerAction(REBOOT_DEVICE, EVENT_AUTO, pRecordFormatEventStr[newGeneralConfig.recordFormatType]);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Put system under watchdog
 * @return  SUCCESS/FAIL
 */
BOOL StartWatchDog(void)
{
    #if defined(RK3568_NVRL)
    if ((GetNvrModelType() == NVR0401XSP2) || (GetNvrModelType() == NVR0801XSP2) || (GetNvrModelType() == NVR1601XSP2))
    {
        /* Open watchdog driver */
        watchdogFd = open("/dev/watchdog_soc", WRITE_ONLY_MODE);
    }
    else
    #endif
    {
        /* Open watchdog driver */
        watchdogFd = open("/dev/watchdog", WRITE_ONLY_MODE);
    }

    /* Is watchdog driver opened? */
    if (watchdogFd == INVALID_FILE_FD)
    {
        EPRINT(SYS_LOG, "fail to open watchdog driver: [err=%s]", STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Feed watchdog
 */
void KickWatchDog(void)
{
    if (watchdogFd == INVALID_FILE_FD)
    {
        /* Nothing to do */
        return;
    }

    #if defined(RK3568_NVRL)
    if ((GetNvrModelType() == NVR0801XP2) || (GetNvrModelType() == NVR1601XP2) || (GetNvrModelType() == NVR1602XP2))
    {
        if (ioctl(watchdogFd, IOCTL_WATCHDOG_TOGGLE, 0) < 0)
        {
            EPRINT(SYS_LOG, "ioctl IOCTL_WATCHDOG_TOGGLE failed");
        }
    }
    else
    #endif
    {
        write(watchdogFd, "M", 1); /* This is dummy value. we have to write some value into file only */
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives output as week name
 * @param   weekDay
 * @return  Week string
 */
CHARPTR GetWeekName(WEEK_DAY_e weekDay)
{
    if (weekDay >= MAX_WEEK_DAYS)
    {
        return NULL;
    }

    return (weekName[weekDay]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives output as month name pointer
 * @param   monthNo
 * @return  Month string
 */
CHARPTR GetMonthName(UINT8 monthNo)
{
    if (monthNo >= MAX_MONTH)
	{
        return NULL;
    }

    return (monthName[monthNo]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API gives output as month number.
 * @param   monStr
 * @return  Month number
 */
UINT8 GetMonthNo(CHARPTR monStr)
{
    UINT8 month;

    for(month = 0; month < MAX_MONTH; month++)
	{
        if(strncmp(monthName[month], monStr, 3) == 0)
		{
			break;
		}
	}
    return month;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function replace charecter from given string.
 * @param   address
 * @param   findChar
 * @param   repChar
 */
void ReplaceCharecter(CHARPTR address, UINT8 findChar, UINT8 repChar)
{
    CHARPTR tempStr;

    while((tempStr = strchr(address, findChar)) != NULL)
	{
		*tempStr = repChar;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function check string length and copy only up to size
 * @param   dst
 * @param   src
 * @param   size
 * @return  SUCCESS /FAIL
 */
BOOL StringNCopy(CHARPTR dst, CHARPTR src, UINT32 size)
{
	UINT32	cpSize = 0;
	UINT32	srcSize;

    dst[0] = '\0';
    if(src == NULL)
	{
        return FAIL;
    }

    srcSize = strlen(src);
    cpSize = (srcSize >= size) ? (size - 1) : srcSize;
    if(cpSize == 0)
    {
        return FAIL;
    }

    strncpy(dst, src, cpSize);
	dst[cpSize] = '\0';
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns the pid number of given process.
 * @param   processName
 * @return  If process is not exists then it returns NILL
 */
INT32 GetPidOfProcess(CHARPTR processName)
{
	INT32			ret, loop;
    INT64			pid;
    DIR				*dir = NULL;
    FILE			*filePtr = NULL;
	CHAR			filePath[FILENAME_MAX];
	CHAR			cmdlineBuf[FILENAME_MAX];
	CHARPTR 		procStr = NULL;
    struct dirent	*entry = NULL;
    struct stat     fileStat;
    UINT32          outLen = 0;

    dir = opendir(PROC_PATH);
    if(dir == NULL)
	{
        return NILL;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        /* Folder must have numeric number to proceed further */
        pid = atol(entry->d_name);
        if (pid <= 0)
        {
            continue;
        }

        outLen = snprintf(filePath, FILENAME_MAX, PROC_PATH"%s", entry->d_name);
        memset(&fileStat, 0, sizeof(struct stat));
        if(stat(filePath, &fileStat) < STATUS_OK)
        {
            continue;
        }

        if(FALSE == S_ISDIR(fileStat.st_mode))
        {
            continue;
        }

        snprintf(filePath + outLen, FILENAME_MAX - outLen, "/cmdline");
        filePtr = fopen(filePath, "r");
        if(filePtr == NULL)
        {
            continue;
        }

        ret = fread(cmdlineBuf, sizeof(CHAR), (FILENAME_MAX - 1), filePtr);
        fclose(filePtr);
        if (ret <= 0)
        {
            continue;
        }

        /* If app/cmd executed with cmd line args then each arg will be separated by null character in /proc/xxx/cmdline */
        cmdlineBuf[ret] = '\0';
        for(loop = 0; loop < (ret - 1); loop++)
        {
            if(cmdlineBuf[loop] == '\0')
            {
                cmdlineBuf[loop] = ' ';
            }
        }

        procStr = strstr(cmdlineBuf, processName);
        if (procStr == NULL)
        {
            continue;
        }

        if (strncmp(procStr, processName, strlen(processName)) != STATUS_OK)
        {
            continue;
        }

        DPRINT(UTILS, "process id found: [name=%s], [pid=%lld]", processName, pid);
        closedir(dir);
        return pid;
    }

    closedir(dir);
    return NILL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function kills the given process.
 * @param   processName
 * @param   sigType
 * @return  SUCCESS / FAIL
 */
BOOL KillProcess(CHARPTR processName, SEND_PROC_SIG_e sigType)
{
    DPRINT(UTILS, "kill process: [name=%s]", processName);
    return KillProcessId(GetPidOfProcess(processName), sigType);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function kills the given process from id.
 * @param   processId
 * @param   sigType
 * @return  SUCCESS / FAIL
 */
BOOL KillProcessId(INT32 processId, SEND_PROC_SIG_e sigType)
{
    INT32 signalType;

    if ((processId == NILL) || (sigType >= MAX_SIG))
    {
        EPRINT(UTILS, "pid not found: [pid=%d], [sigType=%d]", processId, MAX_SIG);
        return FAIL;
    }

    switch(sigType)
    {
        case INT_SIG:
            signalType = SIGINT;
            break;

        case KILL_SIG:
            signalType = SIGKILL;
            break;

        case SIG_USR:
            signalType = SIGUSR1;
            break;

        default:
            return FAIL;
    }

    if (kill(processId, signalType) != STATUS_OK)
    {
        EPRINT(UTILS, "fail to kill process: [pid=%d], [sigType=%d]", processId, MAX_SIG);
        return FAIL;
    }

    DPRINT(UTILS, "kill process: [pid=%d], [sigType=%d]", processId, MAX_SIG);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives first RTP port which is free. And it will also check that its next
 *          position must be free for RTCP.
 * @param   rtpPortPtr
 * @return  SUCCESS / FAIL
 */
BOOL FindFreeRtpPort(UINT16PTR rtpPortPtr)
{
    UINT8   temp2, temp3;
    UINT16  rtpPort, temp1;
    UINT32  cnt;

    MUTEX_LOCK(rtpPortInfo.rtpPortMutex);
	rtpPort = rtpPortInfo.rtpPort.start;
    if((rtpPort % 2) != 0)
	{
		rtpPort++;
	}

	do
	{
		temp1 = (rtpPort / 8);
		temp2 = (rtpPort % 8);

		for(cnt = temp2; cnt < 8; cnt += 2)
		{
			temp3 = (1 << cnt);
            if((rtpPortInfo.rtpPortNo[temp1] & temp3) != 0)
			{
                continue;
            }

            *rtpPortPtr = ((temp1 * 8) + cnt);
            rtpPortInfo.rtpPortNo[temp1] |= temp3;
            rtpPortInfo.rtpPortNo[temp1] |= (temp3 << 1);
            MUTEX_UNLOCK(rtpPortInfo.rtpPortMutex);
            return SUCCESS;
		}

        rtpPort += (8 - temp2);

    } while(rtpPort < rtpPortInfo.rtpPort.end);
    MUTEX_UNLOCK(rtpPortInfo.rtpPortMutex);

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function clears RTP port range from list and makes it available for other request.
 * @param   rtpPortPtr
 */
void ClearRtpPort(UINT16PTR rtpPortPtr)
{
	UINT8		temp2, temp3;
	UINT16		rtpPort, temp1;

    MUTEX_LOCK(rtpPortInfo.rtpPortMutex);
	rtpPort = *rtpPortPtr;
    if(rtpPort == 0)
	{
        MUTEX_UNLOCK(rtpPortInfo.rtpPortMutex);
        return;
    }

    temp1 = (rtpPort / 8);
    temp2 = (rtpPort % 8);
    temp3 = (1 << temp2);

    if (temp1 >= MAX_RTP_PORT_COUNT)
    {
        MUTEX_UNLOCK(rtpPortInfo.rtpPortMutex);
        return;
    }

    rtpPortInfo.rtpPortNo[temp1] &= ~temp3;
    rtpPortInfo.rtpPortNo[temp1] &= ~(temp3 << 1);
    *rtpPortPtr = 0;
    MUTEX_UNLOCK(rtpPortInfo.rtpPortMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get IP Address From Domain name
 * @param   domainName
 * @param   ipAddrType
 * @param   resolvedIpAddr
 * @return  SUCCESS / FAIL
 */
BOOL GetIpAddrFromDomainName(CHARPTR domainName, IP_ADDR_TYPE_e ipAddrType, CHARPTR resolvedIpAddr)
{
    UINT32  status;
    CHAR    ipv4Addr[IPV4_ADDR_LEN_MAX] = { '\0' };
    CHAR    ipv6Addr[IPV6_ADDR_LEN_MAX] = { '\0' };

    /* Resolve Ip Address from domain name based on family */
    if (IP_ADDR_TYPE_IPV4 == ipAddrType)
    {
        status = NMIpUtil_ResolveDomainName(domainName, ipv4Addr, NULL);
    }
    else if (IP_ADDR_TYPE_IPV6 == ipAddrType)
    {
        status = NMIpUtil_ResolveDomainName(domainName, NULL, ipv6Addr);
    }
    else
    {
        status = NMIpUtil_ResolveDomainName(domainName, ipv4Addr, ipv6Addr);
    }

    /* PARASOFT : Rule CERT_C-STR31-b - address won't be NULL if SUCCESS returned */
    if (status == SUCCESS)
    {
        if (ipv4Addr[0] != '\0')
        {
            snprintf(resolvedIpAddr, IPV4_ADDR_LEN_MAX, "%s", ipv4Addr);
        }
        else
        {
            snprintf(resolvedIpAddr, IPV6_ADDR_LEN_MAX, "%s", ipv6Addr);
        }
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function calls PowerAction for System Shutdown/ Restart, and Delete respective timer.
 * @param   data
 */
static void sysPowerActionTimerCb(UINT32 data)
{
    SetPowerAction(data);
    DeleteTimer(&powerActionInfo.timer);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function starts timer for handling System Power Action after some time. If successfully
 *          started timer It writes LOG_SHUTDOWN Live Event. so that all logged-in clients get ack of Power Action.
 * @param   powerAction
 * @param   eventStatus
 * @param   userName
 * @return  SUCCESS / FAIL
 */
BOOL PrepForPowerAction(POWER_ACTION_e powerAction, LOG_EVENT_STATE_e eventStatus, const CHARPTR userName)
{
    LOG_EVENT_SUBTYPE_e evtSubType;
    TIMER_INFO_t        timerInfo;
    CHAR                detail[MAX_EVENT_DETAIL_SIZE];
    static CHAR         advanceDetails[MAX_EVENT_ADVANCE_DETAIL_SIZE] = {0};

	// validate power Action
    if (powerAction >= POWER_ACTION_MAX)
	{
        return FAIL;
    }

    evtSubType = (powerAction == SHUTDOWN_DEVICE) ? LOG_SHUTDOWN : LOG_RESTART;
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(SYS_SHUTDOWN_WAIT_TIME);
    timerInfo.data = powerAction;
    timerInfo.funcPtr = sysPowerActionTimerCb;

    /* Check if somebody has already started Timer */
    MUTEX_LOCK(powerActionInfo.mutex);
    if (powerActionInfo.timer == INVALID_TIMER_HANDLE)
    {
        if (StartTimer(timerInfo, &powerActionInfo.timer) != SUCCESS)
        {
            MUTEX_UNLOCK(powerActionInfo.mutex);
            DPRINT(SYS_LOG, "fail to start power action timer: [action=%s], [user=%s]", powerActStr[powerAction], userName);
            return FAIL;
        }
        MUTEX_UNLOCK(powerActionInfo.mutex);

        snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", eventStatus);
        snprintf(advanceDetails, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", (userName == NULL) ? "System" : userName);
        WriteEvent(LOG_SYSTEM_EVENT, evtSubType, detail, advanceDetails, EVENT_ALERT);
    }
    else
    {
        MUTEX_UNLOCK(powerActionInfo.mutex);

        snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", eventStatus);
        if (userName == NULL)
        {
            if (strcmp(advanceDetails, "System") != 0)
            {
                snprintf(advanceDetails, MAX_EVENT_ADVANCE_DETAIL_SIZE, "System");
                WriteEvent(LOG_SYSTEM_EVENT, evtSubType, detail, "System", EVENT_ALERT);
            }
        }
        else
        {
            if (strcmp(advanceDetails, userName) != 0)
            {
                snprintf(advanceDetails, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", userName);
                WriteEvent(LOG_SYSTEM_EVENT, evtSubType, detail, userName, EVENT_ALERT);
            }
        }
    }

    DPRINT(SYS_LOG, "power action set: [action=%s], [user=%s]", powerActStr[powerAction], userName);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function copy source file to destination file using manual FILE I/O function.
 * @param   sourceFile
 * @param   destFile
 * @param   delay
 * @param   callback
 * @param   userData
 * @return
 */
BOOL CopyFile(CHARPTR sourceFile, CHARPTR destFile, UINT32 delay, COPY_ABORT_CB callback, UINT32 userData)
{
    BOOL        retVal = FAIL;
    UINT8       buff[FILE_RD_WR_CHUNK_SIZE_MAX];
    INT32       sourceFileFd = INVALID_FILE_FD;
    INT32       destFileFd = INVALID_FILE_FD;
    INT32       rdWrSize = 0;
    UINT64      chunkRdWrSize;
    INT64       copySize = 0;
    struct stat statInfo;

    if(stat(sourceFile, &statInfo) < STATUS_OK)
    {
        EPRINT(UTILS, "fail to get file size: [path=%s], [err=%s]", sourceFile, STR_ERR);
        return FAIL;
    }

    sourceFileFd = open(sourceFile, READ_WRITE_SYNC_MODE, USR_RWE_GRP_RE_OTH_RE);
    if(sourceFileFd == INVALID_FILE_FD)
    {
        EPRINT(UTILS, "fail to open file: [path=%s], [err=%s]", sourceFile, STR_ERR);
        return FAIL;
    }

	do
	{
        destFileFd = open(destFile, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
		if(destFileFd == INVALID_FILE_FD)
		{
            EPRINT(UTILS, "fail to open file: [path=%s], [err=%s]", destFile, STR_ERR);
            break;
		}

        DPRINT(UTILS, "copy start: [src=%s], [dst=%s]", sourceFile, destFile);
		retVal = SUCCESS;

		while(copySize < statInfo.st_size)
		{
            chunkRdWrSize = statInfo.st_size - copySize;
            if (chunkRdWrSize > FILE_RD_WR_CHUNK_SIZE_MAX)
            {
                chunkRdWrSize = FILE_RD_WR_CHUNK_SIZE_MAX;
            }

			rdWrSize = read(sourceFileFd, buff, chunkRdWrSize);
			if(rdWrSize != (INT32)chunkRdWrSize)
			{
                EPRINT(UTILS, "fail to read file: [path=%s], [err=%s]", sourceFile, STR_ERR);
				retVal = FAIL;
				break;
			}

			rdWrSize = write(destFileFd, buff, chunkRdWrSize);
			if(rdWrSize != (INT32)chunkRdWrSize)
			{
                EPRINT(UTILS, "fail to write file: [path=%s], [err=%s]", destFile, STR_ERR);
				retVal = FAIL;
				break;
			}

			copySize += chunkRdWrSize;
            if(callback != NULL)
            {
                if(callback(userData) == TRUE)
                {
                    EPRINT(UTILS, "user has aborted file copy: [src=%s], [dst=%s]", sourceFile, destFile);
                    retVal = FAIL;
                    break;
                }
            }
            usleep(delay);
		}

    } while(0);

    CloseFileFd(&sourceFileFd);
    CloseFileFd(&destFileFd);
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is add linux user to the system
 * @param   username
 * @param   password
 * @return  SUCCESS/FAIL
 */
BOOL AddLinuxUser(CHARPTR username, CHARPTR password)
{
    CHAR sysCmd[256];

    snprintf(sysCmd, sizeof(sysCmd), "echo -e \"%s\n%s\n\" | adduser -H -h " MEDIA_DIR_PATH " %s", password, password, username);
    if(FALSE == ExeSensitiveInfoSysCmd(sysCmd))
    {
        EPRINT(UTILS, "fail to add linux user in system: [username=%s]", username);
        return FAIL;
    }

    DPRINT(UTILS, "user added as linux user in system: [username=%s]", username);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is for delete linux user from the system
 * @param   username
 * @param   usrIndx
 * @return  SUCCESS/FAIL
 */
BOOL DeleteLinuxUser(CHARPTR username, UINT8 usrIndx)
{
    CHAR sysCmd[256];

    snprintf(sysCmd, sizeof(sysCmd), "deluser %s", username);
    if(FALSE == ExeSysCmd(TRUE, sysCmd))
    {
        EPRINT(UTILS, "fail to delete linux user from system: [username=%s]", username);
        return FAIL;
    }

    DPRINT(UTILS, "user removed as linux user from system: [username=%s]", username);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function deletes the clients (e.g. Device client) layout files from system using userIndex
 * @param   userIdx
 */
void DeleteClientDispLayoutFile(UINT8 userIdx)
{
    CHAR layoutFile[200];

    /* Prepare file path */
    USERWISE_LAYOUT_FILE(layoutFile, userIdx);

    /* Check file present or not */
    if (access(layoutFile, F_OK))
    {
        return;
    }

    /* File is present. So remove it */
    if (unlink(layoutFile))
    {
        /* Failed to remove file */
        EPRINT(UTILS, "fail to remove file client layout file: [file=%s]", layoutFile);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is FTP service start/stop function
 * @param   port
 * @param   action
 * @param   addport
 * @return  SUCCESS/FAIL
 */
BOOL FtpServiceStartStop(UINT16 port, BOOL action)
{
#if !defined(RK3588_NVRH)
    CHAR    ftpConfigFile[500];
    FILE    *pFtpCnfg;

    /* remove old config file */
    if (access(FTP_FILE_NAME, F_OK) == 0)
    {
        remove(FTP_FILE_NAME);
    }

    /* write  new config file */
    pFtpCnfg = fopen(FTP_FILE_NAME, "w+");
    if (pFtpCnfg != NULL)
    {
        snprintf(ftpConfigFile, sizeof(ftpConfigFile), "%s%d", FTP_CONFIG_FILE, port);
        fwrite(ftpConfigFile, 1, strlen(ftpConfigFile), pFtpCnfg);
        fclose(pFtpCnfg);
    }

    switch(action)
    {
        case START:
        {
            /* if service is already running */
            if (GetPidOfProcess("vsftpd") != NILL)
            {
                /* stop demon */
                if (FALSE == ExeSysCmd(TRUE, FTP_STOP_CMD))
                {
                    return FAIL;
                }

                /* wait for demon to stop */
                DPRINT(UTILS, "ftp service stopped, waiting to restart");
                sleep(1);
            }

            /* start demon */
            if (FALSE == ExeSysCmd(TRUE, FTP_START_CMD))
            {
                return FAIL;
            }

            DPRINT(UTILS, "ftp service start");
        }
        break;

        case STOP:
        {
            /* stop demon */
            if (FALSE == ExeSysCmd(TRUE, FTP_STOP_CMD))
            {
                return FAIL;
            }

            DPRINT(UTILS, "ftp service stop");
        }
        break;

        default:
        {
            /* Invalid action */
        }
        return FAIL;
    }
#endif
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is webserver service start/stop function
 * @param   action
 * @param   webport
 * @return  SUCCESS/FAIL
 */
BOOL WebServerServiceStartStop(BOOL action, UINT16 webport)
{
    switch(action)
    {
        case START:
        {
            CHAR sysCmd[SYS_CMD_MSG_LEN_MAX];

            /* Start web service */
            snprintf(sysCmd, sizeof(sysCmd), WEBSERVER_START_CMD, webport);
            if (FALSE == ExeSysCmd(TRUE, sysCmd))
            {
                return FAIL;
            }

            DPRINT(UTILS, "web service start");
        }
        break;

        case STOP:
        {
            if (FALSE == ExeSysCmd(TRUE, WEBSERVER_STOP_CMD))
            {
                return FAIL;
            }

            DPRINT(UTILS, "web service stop");
        }
        break;

        default:
        {
            /* Invalid action */
        }
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is function update cpu status parameter like CPU Load, load on each Network interface.
 */
void updateCpuStatusParam(void)
{
	JIFFY_COUNTS_t				currJifCnt = { 0 };
    UINT64						totalDiff;
    NETWORK_PORT_e              ifacePort;
    NET_INTERFACE_INFO_PARAM_t	currInfo[NETWORK_PORT_MAX] = { {0} };

	if(SUCCESS != readCpuJiffy(&currJifCnt))
	{
        EPRINT(UTILS, "fail to read cpu jiffy count");
	}

	totalDiff = (currJifCnt.total - statusParam.cpuLoad.prevTotalCnt);
    if (totalDiff == 0)
    {
        totalDiff = 1;
    }

    // CPu Busy Percent = (100 * (Curr Busy Count - Prev Busy Count) / (Curr Total Count - Prev Total Count))
    statusParam.cpuLoad.busyPercent = (UINT8)(( 100 * (currJifCnt.busy - statusParam.cpuLoad.prevBusyCnt)) / (float)totalDiff);
	statusParam.cpuLoad.prevBusyCnt = currJifCnt.busy;
	statusParam.cpuLoad.prevTotalCnt = currJifCnt.total;

	if(SUCCESS != getAllNetworkInterfaceInfo(currInfo))
	{
        EPRINT(UTILS, "fail to get all nw interface info");
	}

    for (ifacePort = NETWORK_PORT_LAN1; ifacePort < NETWORK_PORT_MAX; ifacePort++)
	{
        if (statusParam.prevInfo[ifacePort].txCnt)
        {
            if (currInfo[ifacePort].txCnt >= statusParam.prevInfo[ifacePort].txCnt)
            {
                /* Get Tx bytes difference between two ticks */
                totalDiff = currInfo[ifacePort].txCnt - statusParam.prevInfo[ifacePort].txCnt;

                /* Interface Load = ((bytes * 8 / 1024) / time) Kbit/s */
                statusParam.ifLoad[ifacePort].txLoad = (UINT32)(((totalDiff / 1024) * 8) / CPU_STATUS_CALC_TIME);
            }
            else
            {
                /* Reset average load cals when overflow occures */
                currInfo[ifacePort].txCnt = 0;
            }

        }

        if (statusParam.prevInfo[ifacePort].rxCnt)
        {
            if (currInfo[ifacePort].rxCnt >= statusParam.prevInfo[ifacePort].rxCnt)
            {
                /* Get Rx bytes difference between two ticks */
                totalDiff = currInfo[ifacePort].rxCnt - statusParam.prevInfo[ifacePort].rxCnt;

                /* Interface Load = ((bytes * 8 / 1024) / time) Kbit/s */
                statusParam.ifLoad[ifacePort].rxLoad = (UINT32)(((totalDiff / 1024) * 8) / CPU_STATUS_CALC_TIME);
            }
            else
            {
                /* Reset average load cals when overflow occures */
                currInfo[ifacePort].rxCnt = 0;
            }
        }

        statusParam.prevInfo[ifacePort].txCnt = currInfo[ifacePort].txCnt;
        statusParam.prevInfo[ifacePort].rxCnt = currInfo[ifacePort].rxCnt;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is function gives Load on given network interface in Kbit/s.
 * @param   networkPort
 * @param   ifLoad
 */
void GetNetworkInterfaceLoad(NETWORK_PORT_e networkPort, NET_INTERFACE_LOAD_PARAM_t *ifLoad)
{
    if (networkPort >= NETWORK_PORT_MAX)
	{
        return;
    }

    ifLoad->rxLoad = statusParam.ifLoad[networkPort].rxLoad;
    ifLoad->txLoad = statusParam.ifLoad[networkPort].txLoad;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is function gives current CPU usage
 * @return  CPU Usage
 */
UINT8 GetCurrCpuUsage(void)
{
	return statusParam.cpuLoad.busyPercent;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is function reads total elapsed seconds since device power on.
 * @return  Total seconds since boot up
 */
UINT64 GetDeviceUpTime(void)
{
	CHAR	lineBuf[20];
	INT32	fd;
    UINT64  uptime = 0;

    fd = open(PROC_UPTIME_FILE, READ_ONLY_MODE);
    if (fd == INVALID_FILE_FD)
	{
        EPRINT(UTILS, "fail to open file: [path=%s], [err=%s]", PROC_UPTIME_FILE, STR_ERR);
        return 0;
    }

    // File Data Format: total_seconds idle_seconds
    if (read(fd, lineBuf, 18) <= 0)
    {
        EPRINT(UTILS, "fail to read file: [path=%s], [err=%s]", PROC_UPTIME_FILE, STR_ERR);
        close(fd);
        return 0;
    }

    lineBuf[19] = '\0';
    if (sscanf(lineBuf, "%lld.%*s", &uptime) != 1)
    {
        EPRINT(UTILS, "fail to scan uptime: [path=%s], [err=%s]", PROC_UPTIME_FILE, STR_ERR);
        close(fd);
        return 0;
    }

    close(fd);
	return uptime;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is function reads cpu jiffiy from proc directory. As we need to count CPU busy percent,
 *          It gives CPU busy jiffies and total jiffies in output.
 * @param   pJiffy
 * @return  SUCCESS/FAIL
 */
static BOOL readCpuJiffy(JIFFY_COUNTS_t *pJiffy)
{
    CHAR	lineBuf[128] = "";
	INT32	fd;
    UINT8   readCnt = 0;

    fd = open(PROC_STAT_FILE, READ_ONLY_MODE);
    if (fd == INVALID_FILE_FD)
	{
        EPRINT(UTILS, "fail to open file: [path=%s], [err=%s]", PROC_STAT_FILE, STR_ERR);
        return FAIL;
    }

    readCnt = read(fd, lineBuf, sizeof(lineBuf) - 1);
    if (readCnt <= 0)
    {
        EPRINT(UTILS, "fail to read file: [path=%s], [err=%s]", PROC_STAT_FILE, STR_ERR);
        close(fd);
        return FAIL;
    }

    lineBuf[readCnt] = '\0';
    if (sscanf(lineBuf, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &pJiffy->usr, &pJiffy->nic, &pJiffy->sys, &pJiffy->idle, &pJiffy->iowait, &pJiffy->irq, &pJiffy->softirq, &pJiffy->steal) < 4)
    {
        EPRINT(UTILS, "fail to scan jiffies: [path=%s]", PROC_STAT_FILE);
        close(fd);
        return FAIL;
    }

    pJiffy->total = (pJiffy->usr + pJiffy->nic + pJiffy->sys + pJiffy->idle + pJiffy->iowait + pJiffy->irq + pJiffy->softirq + pJiffy->steal);
    pJiffy->busy = (pJiffy->total - pJiffy->idle - pJiffy->iowait);
    close(fd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is function reads current Tx Rx bytes from proc/net directory.
 * @param   ifInfo
 * @return  SUCCESS/FAIL
 */
static BOOL getAllNetworkInterfaceInfo(NET_INTERFACE_INFO_PARAM_t *ifInfo)
{
    CHARPTR         procline = NULL;
    CHAR            interfaceName[NETWORK_PORT_MAX][INTERFACE_NAME_LEN_MAX];
    BOOL            retVal = FAIL;
    FILE            *pFile;
    CHARPTR         searchPtr = NULL;
    size_t          readCount;
    NETWORK_PORT_e  ifacePort;

    /* Open the file for network statistics */
    pFile = fopen(PROC_NET_DEV_FILE, "r");
    if (pFile == NULL)
	{
        EPRINT(UTILS, "fail to open file: [path=%s], [err=%s]", PROC_NET_DEV_FILE, STR_ERR);
        return FAIL;
    }

    memset(&interfaceName, 0, sizeof(interfaceName));
    for (ifacePort = NETWORK_PORT_LAN1; ifacePort < NETWORK_PORT_MAX; ifacePort++)
    {
        GetNetworkPortName(ifacePort, interfaceName[ifacePort]);
    }

    /*  File Data Format
        Inter-|   Receive                                                |  Transmit
        face  |  bytes packets errs drop fifo frame compressed multicast |  bytes packets errs drop fifo colls carrier compressed
        lo:          0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
        bond0:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
        eth0:  1585668    2059    0    0    0     0          0         0   125437     561    0    0    0     0       0          0
        eth1:        0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
     */

    /* Go to line containing eth0 information */
    while (getline(&procline, &readCount, pFile) > 0)
    {
        for (ifacePort = NETWORK_PORT_LAN1; ifacePort < NETWORK_PORT_MAX; ifacePort++)
        {
            /* Skip invalid interface */
            if (interfaceName[ifacePort][0] == '\0')
            {
                continue;
            }

            searchPtr = strstr(procline, interfaceName[ifacePort]);
            if (searchPtr == NULL)
            {
                continue;
            }

            ifInfo[ifacePort].rxCnt = 10;
            ifInfo[ifacePort].txCnt = 10;
            searchPtr += (strlen(interfaceName[ifacePort]) + 1);

            // Read Only Tx Rx Bytes
            if (sscanf(searchPtr, "%lld %*s %*s %*s %*s %*s %*s %*s %lld %*s", &ifInfo[ifacePort].rxCnt, &ifInfo[ifacePort].txCnt) != 2)
            {
                EPRINT(UTILS, "fail to scan network load: [path=%s], [interface=%s]", PROC_NET_DEV_FILE, interfaceName[ifacePort]);
                break;
            }

            retVal = SUCCESS;
            break;
        }
    }

    free(procline);
    fclose(pFile);
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function parse FTP LIST command response. It gives one entry from list response,
 *          give information about whether the entry is directory is not. And it increments the
 *          source buffer to point to next entry.
 * @param   list
 * @param   entry
 * @param   maxSize
 * @param   isDir
 * @return  SUCCESS / FAIL
 */
BOOL ParseFTPListReply(CHARPTR *list, CHARPTR entry, UINT16 maxSize, BOOL *isDir)
{
	BOOL	retVal = FAIL;
	CHARPTR startPtr;
	CHARPTR endPtr;
	UINT8 	noOfSpaceFound;

    /* UNIX System Directory List
        drwxrwxr-x    2 0        0             936 Jun 01 19:04 bin
        drwxrwxr-x    3 0        0             304 May 02 04:04 config
        drwxrwxr-x    2 0        0             304 May 14 09:32 event
        drwxrwxr-x    2 0        0             232 Mar 11 05:41 lib
        -rwxrwxr-x    1 0        0           16848 Apr 23 13:48 sysUpgrade.bin
        -rwxrwxr-x    1 0        0          101299 May 03 04:01 vnstat
        lrwx------    1 root     root            64 Jun  1 19:08 0 -> /dev/null */
    if ((list == NULL) || ((*list) == NULL))
	{
        return FAIL;
    }

    do
    {
        retVal = FAIL;
        startPtr = *list;
        noOfSpaceFound = 0;

        // Reach to Directory Name First, as seen in LIST response directory name is after NO_OF_SPACE_TO_GET_NAME white spaces
        while (noOfSpaceFound != NO_OF_SPACE_TO_GET_NAME)
        {
            if (startPtr == NULL)
            {
                break;
            }

            if ((startPtr = strchr(startPtr, ' ')) == NULL)
            {
                break;
            }

            // Ignore all consecutive white space
            noOfSpaceFound++;
            while (startPtr[0] == ' ')
            {
                startPtr++;
            }
        }

        if (noOfSpaceFound != NO_OF_SPACE_TO_GET_NAME)
        {
            continue;
        }

        // get End Of Line
        endPtr = strchr(startPtr, '\n');
        if (endPtr == NULL)
        {
            continue;
        }

        // Check if enough space to store entry
        if ((endPtr - startPtr) >= maxSize)
        {
            continue;
        }

        memcpy(entry, startPtr, (endPtr - startPtr));
        entry[(endPtr - startPtr)] = '\0';

        // As seen in LIST reply For Directory first character is 'd'
        if (*list[0] == 'd')
        {
            entry[(endPtr - startPtr)] = '/';
            entry[(endPtr - startPtr) + 1] = '\0';
            *isDir = TRUE;
        }
        else
        {
            *isDir = FALSE;
        }

        retVal = SUCCESS;
        // Increment list pointer to point to next line
        *list = (endPtr + 1);

        if ((strcmp(entry, "./") == STATUS_OK) || (strcmp(entry, "../") == STATUS_OK))
        {
            continue;
        }

        break;

    } while (retVal == SUCCESS);

	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ParseIpAddressGetIntValue
 * @param   ipAddrstr
 * @param   allOctect
 */
void ParseIpAddressGetIntValue(CHARPTR ipAddrstr, UINT8PTR allOctect)
{
    INT32   tempOctet[4];
    UINT8   index;

    sscanf(ipAddrstr, "%d.%d.%d.%d", &tempOctet[0], &tempOctet[1], &tempOctet[2], &tempOctet[3]);
	for(index = 0; index < 4; index++)
	{
		*(allOctect + index) = (UINT8)tempOctet[index];
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   UnPackGridGeneral
 * @param   input
 * @param   col
 * @param   raw
 * @param   output
 * @param   inputLength
 */
void UnPackGridGeneral(CHARPTR input, UINT8 col, UINT8 raw, CHARPTR *output, UINT16 inputLength)
{
	UINT16	byteIndex = 0;
	UINT8	bitCount = 0, tempBitNo = 0;
	UINT8	tempOutput[(raw *col) + 8];
	UINT16	tempFillCount = 0, rowCount = 0, colCount = 0;

    memset(tempOutput, 0, sizeof(tempOutput));
	for(byteIndex = 0; byteIndex < inputLength; byteIndex++)
	{
		for(bitCount = 0; bitCount < 8; bitCount++)
		{
			tempBitNo = (7 - bitCount);
			if((1 << tempBitNo) & (UINT8)input[byteIndex])
			{
				tempOutput[tempFillCount++] = 1;
			}
			else
			{
				tempOutput[tempFillCount++] = 0;
			}
		}
	}

	tempFillCount = 0;
	for(rowCount = 0; rowCount < raw; rowCount++)
	{
		for(colCount = 0; colCount < col; colCount++)
		{
			output[rowCount][colCount] = tempOutput[tempFillCount++];
		}
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function convert rendomgrid into 44X36 grid.
 * @param   rendomGridPtr
 * @param   grid44_36Ptr
 * @param   column
 * @param   row
 */
void ConvertUnpackedGridTo44_36(CHARPTR *rendomGridPtr, CHARPTR *grid44_36Ptr, UINT8 column, UINT8 row)
{
	UINT8 unitBoxColumn = 0, unitBoxRow = 0;
	UINT8 unitBoxstartColumn = 0, unitBoxstartRow = 0;
    UINT8 rowCnt = 0, tempRowCnt = 0;
	UINT8 columnCnt = 0, tempColumnCnt = 0;

	uninRowColumnBoxes(&unitBoxColumn, &unitBoxstartColumn, &unitBoxRow, &unitBoxstartRow, row, column);

    for(rowCnt = 0; rowCnt < row; rowCnt++)
	{
        for(columnCnt = 0; columnCnt  < column; columnCnt++)
		{
            for(tempColumnCnt = 0; tempColumnCnt != unitBoxColumn; tempColumnCnt++)
			{
                if(((rowCnt*unitBoxstartRow) < 36) && ((columnCnt*unitBoxstartColumn)+tempColumnCnt  >= 44))
				{
					break;
				}

                for(tempRowCnt = 0; tempRowCnt != unitBoxRow; tempRowCnt++)
                {
                    if(((rowCnt*unitBoxstartRow)+tempRowCnt >= 36) && ((columnCnt*unitBoxstartColumn)+tempColumnCnt  < 44))
                    {
                        break;
                    }

                    grid44_36Ptr[(rowCnt*unitBoxstartRow)+tempRowCnt][(columnCnt*unitBoxstartColumn)+tempColumnCnt] = rendomGridPtr[rowCnt][columnCnt];
                }
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function convert 44X36 grid into rendomgrid.
 * @param   grid44_36Ptr
 * @param   rendomGridPtr
 * @param   column
 * @param   row
 */
void Convert44_36ToUnpackedGrid(CHARPTR *grid44_36Ptr, CHARPTR *rendomGridPtr, UINT8 column, UINT8 row)
{
	UINT8 unitBoxColumn = 0, unitBoxRow = 0;
    UINT8 unitBoxstartColumn = 0, unitBoxstartRow = 0;
    UINT8 rowCnt = 0, tempRowCnt = 0;
    UINT8 columnCnt = 0, tempColumnCnt = 0;
	BOOL  currentBlockStatus = INACTIVE;

	uninRowColumnBoxes(&unitBoxColumn, &unitBoxstartColumn, &unitBoxRow, &unitBoxstartRow, row, column);

    for(rowCnt = 0; rowCnt < row; rowCnt++)
	{
        for(columnCnt = 0; columnCnt < column; columnCnt++)
		{
            rendomGridPtr[rowCnt][columnCnt] = 0;
			currentBlockStatus = INACTIVE;

			for(tempRowCnt = 0; tempRowCnt < unitBoxRow; tempRowCnt++)
			{
				for(tempColumnCnt = 0; tempColumnCnt < unitBoxColumn; tempColumnCnt++)
				{
					if(grid44_36Ptr[(rowCnt * unitBoxstartRow) + tempRowCnt][(columnCnt * unitBoxstartColumn) + tempColumnCnt] == 1)
					{
						currentBlockStatus = ACTIVE;
						break;
					}
				}

				if(currentBlockStatus == ACTIVE)
				{
                    rendomGridPtr[rowCnt][columnCnt] = 1;
					break;
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will calculate unit boxRow, unitBoxcolumn
 * @param   unitBoxColumn
 * @param   unitBoxstartColumn
 * @param   unitBoxRow
 * @param   unitBoxstartRow
 * @param   row
 * @param   column
 */
static void uninRowColumnBoxes(UINT8PTR unitBoxColumn ,UINT8PTR unitBoxstartColumn, UINT8PTR unitBoxRow, UINT8PTR unitBoxstartRow, UINT8 row, UINT8 column)
{
	if(44%column != 0)
	{
		*unitBoxColumn = ((44/column) + 1);
	}
	else
	{
		*unitBoxColumn = 44/column;
	}

	if(36%row != 0)
	{
		*unitBoxRow = ((36/row)+1);
	}
	else
	{
		*unitBoxRow = 36/row;
	}

	*unitBoxstartColumn = 44/column;
	*unitBoxstartRow = 36/row;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function convert 2D buffer into packed string.
 * @param   convertedBuf - 2D buffer cwhich will be packed
 * @param   col - column span of buffer
 * @param   raw - row span of buffer
 * @param   packgrid - packed output
 * @param   OutputLen - packed output legnth
 */
void PackGridGeneral(CHARPTR *convertedBuf, UINT8 col, UINT8 raw, CHARPTR packgrid, UINT16PTR OutputLen)
{
    UINT8 	rowCnt, colCnt, tempCnt = 0;
    UINT8 	count = 8;

    packgrid[tempCnt] = 0;
    for (rowCnt = 0; rowCnt < raw; rowCnt++)
    {
        for (colCnt = 0; colCnt < col; colCnt++)
        {
            if (count == 0)
            {
                tempCnt++;
                packgrid[tempCnt] = 0;
                count = 8;
            }

            count--;
            if (convertedBuf[rowCnt][colCnt] == 1)
            {
                packgrid[tempCnt] |= (UINT8)(0x01 << count);
            }
        }
    }

    *OutputLen = (tempCnt + 1);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function allocte memory for 2D array.
 * @param   rows - column span of buffer
 * @param   cols - row span of buffer
 * @param   elementSize - size of single element
 * @return  FAIL/SUCCESS
 */
void **Allocate2DArray(UINT16 rows, UINT16 cols, UINT32 elementSize)
{
    UINT16 rowCnt, freeCnt;

    /* Nothing to do if inputs are invalid */
    if ((rows == 0) || (cols == 0) || (elementSize == 0))
    {
        return NULL;
    }

    /* Allocate rows for 2D array */
    /* PARASOFT : Rule CERT_C-ARR01-a marked as false positive */
    void **matrix = (void **)calloc(1, rows * sizeof(void*));
    if (matrix == NULL)
    {
        return NULL;
    }

    for (rowCnt = 0; rowCnt < rows; rowCnt++)
    {
        /* Allocate columns in a row */
        matrix[rowCnt] = calloc(1, cols * elementSize);
        if (matrix[rowCnt] == NULL)
        {
            break;
        }
    }

    /* Free previously allocated memory */
    if (rowCnt < rows)
    {
        for (freeCnt = 0; freeCnt < rowCnt; freeCnt++)
        {
            FREE_MEMORY(matrix[freeCnt]);
        }

        FREE_MEMORY(matrix);
        return NULL;
    }

    return matrix;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Free memory of 2D array
 * @param   memory
 * @param   rows
 */
void Free2DArray(void **matrix, UINT16 rows)
{
    UINT16 rowCnt;

    /* Nothing to do with null pointer */
    if (NULL == matrix)
    {
        return;
    }

    for (rowCnt = 0; rowCnt < rows; rowCnt++)
    {
        /* Free allocated columns in a row */
        FREE_MEMORY(matrix[rowCnt]);
    }

    /* Free allocated rows */
    FREE_MEMORY(matrix);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ValidateIpSubnetWithLan1
 * @param   startIp
 * @param   endIp
 * @return  TRUE / FALSE
 */
BOOL ValidateIpSubnetWithLan1(CHARPTR startIp, CHARPTR endIp)
{
    UINT32          tempDevIpAddr = 0;
    UINT32          tmpCheckIp = 0;
    UINT32          tempLanSubNet = 0;
    INT32           bytes[4];
    LAN_CONFIG_t    lanCfg[MAX_LAN_PORT];

	// check if auto config ip range is in Lan1 subnet...
	ReadLan1Config(&lanCfg[LAN1_PORT]);
    sscanf(lanCfg[LAN1_PORT].ipv4.lan.ipAddress, "%d.%d.%d.%d", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
    tempDevIpAddr = ((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3]));

    sscanf(lanCfg[LAN1_PORT].ipv4.lan.subnetMask, "%d.%d.%d.%d", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
    tempLanSubNet = ((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3]));

    sscanf(startIp, "%d.%d.%d.%d", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
    tmpCheckIp = ((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3]));

    if((tempDevIpAddr & tempLanSubNet) != (tmpCheckIp & tempLanSubNet))
	{
        return FALSE;
    }

    sscanf(endIp, "%d.%d.%d.%d", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
    tmpCheckIp = ((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3]));
    if((tempDevIpAddr & tempLanSubNet) != (tmpCheckIp & tempLanSubNet))
    {
        return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is timer callback to monitor the system status (e.g. CPU temperature, CPU load, network stats etc)
 * @param   data
 */
static void systemStatusMonitorTmrCb(UINT32 data)
{
    static BOOL     isBuzzerStarted = FALSE;
    static UINT8    deBounceCnt = 0;
    static UINT8    cpuStatusUpdateCnt = 0;
    UINT32          cpuTempCels = GetCpuTemperature();

    /* Update CPU status timer counter and check for expiry */
    cpuStatusUpdateCnt++;
    if (cpuStatusUpdateCnt >= CPU_STATUS_CALC_TIME)
    {
        updateCpuStatusParam();
        cpuStatusUpdateCnt = 0;
    }

    if (isBuzzerStarted == FALSE)
    {
        if (cpuTempCels < CPU_MAX_TEMP_FOR_BUZZER)
        {
            /* No issue in cpu temperature */
            deBounceCnt = 0;
            return;
        }
    }
    else
    {
        if ((cpuTempCels >= CPU_MAX_TEMP_FOR_BUZZER) && (cpuTempCels < CPU_MAX_TEMP_FOR_SHUTDOWN))
        {
            EPRINT(UTILS, "cpu temperature is critically high: [celsius=%d], [deBounceCnt=%d]", cpuTempCels, deBounceCnt);
            deBounceCnt = 0;
            return;
        }
    }

    deBounceCnt++;
    if(deBounceCnt < CPU_TEMP_DEBOUNCE_COUNT)
    {
        WPRINT(UTILS, "cpu temperature in radar: [celsius=%d], [deBounceCnt=%d]", cpuTempCels, deBounceCnt);
        return;
    }

    deBounceCnt = 0;
    DPRINT(UTILS, "cpu temperature debounce occurred: [celsius=%d]", cpuTempCels);
    if(cpuTempCels >= CPU_MAX_TEMP_FOR_SHUTDOWN)
    {
        StartStopCpuTempBuzz(STOP);
        WriteEvent(LOG_OTHER_EVENT, LOG_BUZZER_STATUS, NULL, CPU_TEMP_ADV_DETAIL, EVENT_STOP);
        PrepForPowerAction(SHUTDOWN_DEVICE, EVENT_AUTO, CPU_TEMP_ADV_DETAIL);
        isBuzzerStarted = FALSE;
    }
    else if(cpuTempCels >= CPU_MAX_TEMP_FOR_BUZZER)
    {
        if(isBuzzerStarted == FALSE)
        {
            //Send Buzzer ringing event and start buzzer
            StartStopCpuTempBuzz(START);
            WriteEvent(LOG_OTHER_EVENT, LOG_BUZZER_STATUS, NULL, CPU_TEMP_ADV_DETAIL, EVENT_START);
            isBuzzerStarted = TRUE;
        }
    }
    else
    {
        if(isBuzzerStarted == TRUE)
        {
            StartStopCpuTempBuzz(STOP);
            WriteEvent(LOG_OTHER_EVENT, LOG_BUZZER_STATUS, NULL, CPU_TEMP_ADV_DETAIL, EVENT_STOP);
            isBuzzerStarted = FALSE;
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function compare two string. if both match then return index compare string.
 * @param   strPtr - string ptr which is compare with all string
 * @param   strBuffPtr - buffer pointer to all string
 * @param   maxIndex - max number of string in given buffer pointer
 * @return  index-index of match string from string buffer
 */
UINT8 ConvertStringToIndex(CHARPTR strPtr, const CHARPTR *strBuffPtr, UINT8 maxIndex)
{
    UINT8 index = 0;

    /* Check upto max index in string buffer */
    while(index < maxIndex)
    {
        /* if match outof loop */
        if((strcasecmp(strPtr, strBuffPtr[index])) == 0)
        {
            /* return match string index */
            return index;
        }
        index++;
    }

    /* return maxindex index */
    return index;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will parse string from source buffer at current pointer till FSP and copy
 *          to destination buffer
 * @param   src - source pointer from which to parse string.
 * @param   delim - delimeter
 * @param   dest - Pointer to destination buffer where parse string is stored.
 * @param   maxDestSize - Length of string buffer to be filled.
 * @return  Returns SUCCESS/FAIL
 */
BOOL ParseStr(CHARPTR *src, UINT8 delim, CHARPTR dest, UINT16 maxDestSize)
{
    CHARPTR sptr, eptr;
    UINT32  length;

    sptr = *src;
    eptr = memchr(*src, delim, maxDestSize);
    if(eptr == NULL)
    {
        return FAIL;
    }

    length = eptr - sptr;
    if(length >= maxDestSize)
    {
        return FAIL;
    }

    strncpy(dest, *src, length);
    dest[length] = '\0';
    *src += (length + 1);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will parse string from source buffer at current pointer till FSP and copy
 *          to destination with max size of buffer
 * @param   src - source pointer from which to parse string.
 * @param   delim - delimeter
 * @param   dest - Pointer to destination buffer where parse string is stored.
 * @param   maxDestSize - Length of string buffer to be filled.
 * @return  Returns SUCCESS/FAIL
 */
BOOL ParseNStr(CHARPTR *src, UINT8 delim, CHARPTR dest, UINT16 maxDestSize)
{
    CHARPTR sptr, eptr;
    UINT32  totalLen, dataLen;

    sptr = *src;
    eptr = memchr(*src, delim, strlen(*src));
    if(eptr == NULL)
    {
        return FAIL;
    }

    dataLen = totalLen = eptr - sptr;
    if (dataLen >= maxDestSize)
    {
        dataLen = maxDestSize - 1;
    }

    strncpy(dest, *src, dataLen);
    dest[dataLen] = '\0';
    *src += (totalLen + 1);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API converts an ASCII string to integer
 * @param   ascii
 * @param   integer
 * @return  Returns SUCCESS/FAIL
 */
BOOL AsciiToInt(CHARPTR ascii, UINT64PTR integer)
{
    UINT32  asciiLen;
    UINT64  multiplicant;

    // check for invalid input/output parameters
    if ((ascii == NULL) || (integer == NULL))
    {
        return FAIL;
    }

    *integer = 0;
    multiplicant = 1;

    // run the loop, length times, and sum up the individual element in integer
    for(asciiLen = strlen(ascii); (asciiLen > 0); asciiLen--)
    {
        /* PARASOFT : No need to check array index */
        // check if the value falls between '0' to '9'
        if((ascii[asciiLen - 1] < '0') || (ascii[asciiLen - 1] > '9'))
        {
            *integer = 0;
            return FAIL;
        }

        *integer += ((ascii[asciiLen - 1] - '0') * multiplicant);
        multiplicant *= 10;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ParseStringGetVal
 * @param   commandStrPtr
 * @param   tempDataPtr
 * @param   maxCmdArg
 * @param   seperator
 * @return  Returns SUCCESS/FAIL
 */
BOOL ParseStringGetVal(CHARPTR *commandStrPtr, UINT64PTR tempDataPtr, UINT8 maxCmdArg, UINT8 seperator)
{
    UINT8   tempArg;
    CHAR    commandArg[255];

    for(tempArg = 0; tempArg < maxCmdArg; tempArg++)
    {
        if(ParseStr(commandStrPtr, seperator, commandArg, 255) == FAIL)
        {
            return FAIL;
        }

        if(AsciiToInt(commandArg, &tempDataPtr[tempArg]) == FAIL)
        {
            return FAIL;
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   encode into base64
 * @param   pPlainData: string to be encoded
 * @param   dataLen
 * @return  encoded string
 */
CHARPTR EncodeBase64(const CHAR *pPlainData, UINT32 dataLen)
{
    CHAR                *pFinal, *pOutput;
    UINT8               outVal;
    INT32               outLen;
    const UINT8         *pInput = (const UINT8*)pPlainData;
    static const CHAR   *pBase64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    if ((dataLen == 0) || (pPlainData == NULL))
    {
        return NULL;
    }

    outLen = GET_PLAIN_TO_BASE64_SIZE(dataLen);
    pFinal = malloc(outLen);
    if (NULL == pFinal)
    {
        return NULL;
    }

    pOutput = pFinal;
    while (dataLen >= 3)
    {
        *pOutput++ = pBase64[pInput[0] >> 2];
        *pOutput++ = pBase64[((pInput[0] << 4) & 0x30) | (pInput[1] >> 4)];
        *pOutput++ = pBase64[((pInput[1] << 2) & 0x3c) | (pInput[2] >> 6)];
        *pOutput++ = pBase64[pInput[2] & 0x3f];
        pInput += 3;
        dataLen -= 3;
    }

    if (dataLen > 0)
    {
        *pOutput++ = pBase64[pInput[0] >> 2];
        outVal = (pInput[0] << 4) & 0x30;
        if (dataLen > 1) outVal |= pInput[1] >> 4;
        *pOutput++ = pBase64[outVal];
        *pOutput++ = (dataLen < 2) ? '=' : pBase64[(pInput[1] << 2) & 0x3c];
        *pOutput++ = '=';
    }

    *pOutput = '\0';
    return pFinal;
}

//------------------------------------------------------------------------------------------
/**
 * @brief   decode from base64
 * @param   pEncData: string to be decoded
 * @param   dataLen
 * @param   pOutLen - Decoded data length
 * @return  decoded string
 */
CHARPTR DecodeBase64(const CHAR *pEncData, UINT32 dataLen, UINT32 *pOutLen)
{
    CHAR    *pOutput, *pFinal;
    INT32   char1, char2, char3, char4;
    UINT32  dataCnt, outLen = 0, outSizeMax;

    if ((dataLen == 0) || (pEncData == NULL))
    {
        return NULL;
    }

    outSizeMax = GET_BASE64_TO_PLAIN_SIZE(dataLen);
    pOutput = malloc(outSizeMax);
    if (pOutput == NULL)
    {
        return NULL;
    }

    pFinal = pOutput;
    if (pEncData[0] == '+' && pEncData[1] == ' ')
    {
        pEncData += 2;
    }

    for (dataCnt = 0; dataCnt < (dataLen / 4); dataCnt++)
    {
        char1 = pEncData[0];
        char2 = pEncData[1];
        char3 = pEncData[2];
        char4 = pEncData[3];

        if ((CHAR64(char1) == -1) || (CHAR64(char2) == -1) || ((char3 != '=') && (CHAR64(char3) == -1)) || ((char4 != '=') && (CHAR64(char4) == -1)))
        {
            free(pFinal);
            return NULL;
        }

        pEncData += 4;
        *pOutput++ = (CHAR64(char1) << 2) | (CHAR64(char2) >> 4);
        if (++outLen >= outSizeMax)
        {
            free(pFinal);
            return NULL;
        }

        if (char3 == '=')
        {
            continue;
        }

        *pOutput++ = ((CHAR64(char2) << 4) & 0xf0) | (CHAR64(char3) >> 2);
        if (++outLen >= outSizeMax)
        {
            free(pFinal);
            return NULL;
        }

        if (char4 == '=')
        {
            continue;
        }

        *pOutput++ = ((CHAR64(char3) << 6) & 0xc0) | CHAR64(char4);
        if (++outLen >= outSizeMax)
        {
            free(pFinal);
            return NULL;
        }
    }

    *pOutput = 0;
    if (pOutLen != NULL)
    {
        *pOutLen = outLen;
    }

    return pFinal;
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Encryption of data using openssl with AES256 method
 * @param   pDataBuff - Data buffer
 * @param   buffLen - Buffer length
 * @param   pPassword - Password to be used for encryption
 * @param   pSaltKey - Saltkey to be used for encryption. It must be 8 bytes long.
 * @param   pOutLen - Output cipher text length
 * @return  Cipher text data buffer. It needs to be freed by caller.
 */
UINT8 *EncryptAes256(const UINT8 *pDataBuff, UINT32 buffLen, const CHAR *pPassword, const CHAR *pSaltKey, UINT32 *pOutLen)
{
    EVP_CIPHER_CTX  *ctx;
    UINT8           key[32];
    UINT8           iv[32];
    INT32           finalLen = 0;
    UINT8           *cipherData;

    /* Generate key and iv from password and saltkey with only 1 iteration */
    if (sizeof(key) != EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), (const UINT8 *)pSaltKey, (const UINT8 *)pPassword, strlen(pPassword), 1, key, iv))
    {
        EPRINT(UTILS, "failed to generate key for enc");
        return NULL;
    }

    /* Allocate memory for cipher data */
    *pOutLen = buffLen + AES_BLOCK_SIZE;  /* max cipherData len: plain text len + AES_BLOCK_SIZE bytes */
    cipherData = malloc(*pOutLen);
    if (NULL == cipherData)
    {
        EPRINT(UTILS, "failed to alloc mem for cipher data");
        return NULL;
    }

    /* Get cipher object */
    ctx = EVP_CIPHER_CTX_new();
    if (NULL == ctx)
    {
        EPRINT(UTILS, "failed to alloc cipher ctx");
        free(cipherData);
        return NULL;
    }

    /* Init cipher object for encryption */
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    /* Update cipherData, outputLen is filled with the length of cipherData generated, len is the size of plainData in bytes */
    EVP_EncryptUpdate(ctx, cipherData, (INT32 *)pOutLen, pDataBuff, buffLen);

    /* Update cipherData with the final remaining bytes */
    EVP_EncryptFinal_ex(ctx, cipherData + *pOutLen, &finalLen);

    /* Store cipher data length */
    *pOutLen += finalLen;

    /* Free cipher object */
    EVP_CIPHER_CTX_free(ctx);

    /* cipherData needs to be freed by caller */
    return cipherData;
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Decryption of data using openssl with AES256 method
 * @param   pDataBuff - Data buffer
 * @param   buffLen - Buffer length
 * @param   pPassword - Password to be used for decryption
 * @param   pSaltKey - Saltkey to be used for decryption. It must be 8 bytes long.
 * @param   pOutLen - Output plain text length
 * @return  Plain text data buffer. It needs to be freed by caller.
 */
UINT8 *DecryptAes256(const UINT8 *pDataBuff, UINT32 buffLen, const CHAR *pPassword, const CHAR *pSaltKey, UINT32 *pOutLen)
{
    EVP_CIPHER_CTX  *ctx;
    UINT8           key[32];
    UINT8           iv[32];
    INT32           finalLen = 0;
    UINT8           *plainData;

    /* Generate key and iv from password and saltkey with only 1 iteration */
    if (sizeof(key) != EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), (const UINT8 *)pSaltKey, (const UINT8 *)pPassword, strlen(pPassword), 1, key, iv))
    {
        EPRINT(UTILS, "failed to generate key for dec");
        return NULL;
    }

    /* Allocate memory for plain data */
    plainData = malloc(buffLen+1);
    if (NULL == plainData)
    {
        EPRINT(UTILS, "failed to alloc mem for plain data");
        return NULL;
    }

    /* Get cipher object */
    ctx = EVP_CIPHER_CTX_new();
    if (NULL == ctx)
    {
        EPRINT(UTILS, "failed to alloc cipher ctx");
        free(plainData);
        return NULL;
    }

    /* Init cipher object for decryption */
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    /* Update plainData, outputLen is filled with the length of dataBuff generated, len is the size of cipherData in bytes */
    EVP_DecryptUpdate(ctx, plainData, (INT32 *)pOutLen, pDataBuff, buffLen);

    /* Update plainData with the final remaining bytes */
    EVP_DecryptFinal_ex(ctx, plainData + *pOutLen, &finalLen);

    /* Store plain data length */
    *pOutLen += finalLen;

    /* Free cipher object */
    EVP_CIPHER_CTX_free(ctx);

    /* Add string termination character */
    plainData[buffLen] = '\0';

    /* plainData needs to be freed by caller */
    return plainData;
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Generate random number
 * @return  random number
 * @note    We have not closed fd to avoid open and close in every random number generation
 */
UINT32 GetRandomNum(void)
{
    UINT32          randNum;
    static INT32    devFd = INVALID_FILE_FD;

    /* Open urandom only once */
    if (devFd == INVALID_FILE_FD)
    {
        /* open urandom file */
        devFd = open("/dev/urandom", O_RDONLY);
    }

    /* If failed to open then generate it from older method */
    if (devFd == INVALID_FILE_FD)
    {
        /* failed to open urandom */
        WPRINT(UTILS, "fail to open /dev/urandom: [err=%s]", STR_ERR);
        randNum = 0;
    }
    else
    {
        /* Read 4 bytes from fd */
        if (-1 == read(devFd, &randNum, sizeof(randNum)))
        {
            /* Failed to get data from urandom */
            randNum = 0;
        }
    }

    /* On failure get from random */
    if (randNum == 0)
    {
        srandom(time(NULL));
        randNum = (UINT32)random();
    }

    /* Provide generated number */
    return randNum;
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Generate random alphanumeric string
 * @param   pString - Pointer to string buffer
 * @param   length - Buffer length including null character
 * @param   onlyHexChar - Only hex characters needed
 * @note    Length should be including null character
 */
void GetRandomAlphaNumStr(CHARPTR pString, UINT8 length, BOOL onlyHexChar)
{
    static UINT32   offset = 0;
    const CHAR      *alphaNum;
    UINT8           alphaNumLen;
    UINT16          charCnt;

    /* Validate input string buffer and input paramters */
    if (pString == NULL)
    {
        return;
    }

    /* Is upper case character needed? */
    if (TRUE == onlyHexChar)
    {
        alphaNum = "abcdef0123456789";
    }
    else
    {
        alphaNum = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    }

    /* Get input string length */
    alphaNumLen = strlen(alphaNum);

    /* Generate required length of string */
    for (charCnt = 0; charCnt < (length - 1); charCnt++)
    {
        /* Generate random chracter */
        pString[charCnt] = alphaNum[(GetRandomNum() + offset) % alphaNumLen];
        offset++;
    }

    /* Terminate string with null */
    pString[charCnt] = '\0';
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Will generate random number required for sending discovery command to camera
 * @param   uuidStr
 */
void GenerateUuidStr(CHARPTR uuidStr)
{
    UINT16 outLen = 0;

    // Format: ff4e34b8-baab-d5bd-ee6c-d55ef21e3565
    GetRandomAlphaNumStr(&uuidStr[outLen], 9, TRUE);
    outLen += 8;
    uuidStr[outLen++] = '-';

    GetRandomAlphaNumStr(&uuidStr[outLen], 5, TRUE);
    outLen += 4;
    uuidStr[outLen++] = '-';

    GetRandomAlphaNumStr(&uuidStr[outLen], 5, TRUE);
    outLen += 4;
    uuidStr[outLen++] = '-';

    GetRandomAlphaNumStr(&uuidStr[outLen], 5, TRUE);
    outLen += 4;
    uuidStr[outLen++] = '-';

    GetRandomAlphaNumStr(&uuidStr[outLen], 13, TRUE);
    outLen += 12;
    uuidStr[outLen++] = '\0';
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Function to remove a list of special characters from a string
 * @param   pSrcStr
 * @param   pSpecialChars
 * @param   pDstStr
 */
void RemoveSpecialCharacters(const CHAR *pSrcStr, const CHAR *pSpecialChars, CHAR *pDstStr)
{
    BOOL    isSpecial;
    UINT16  strIdx, specialCharIdx;
    UINT16  strLen = strlen(pSrcStr);
    UINT16  specialCharLen = strlen(pSpecialChars);
    UINT16  outIdx = 0;

    /* Check all characters of the string */
    for (strIdx = 0; strIdx < strLen; strIdx++)
    {
        /* Check for all special characters */
        isSpecial = FALSE;
        for (specialCharIdx = 0; specialCharIdx < specialCharLen; specialCharIdx++)
        {
            /* Check if current character in string is a special character */
            if (pSrcStr[strIdx] == pSpecialChars[specialCharIdx])
            {
                /* It is special characters */
                isSpecial = TRUE;
                break;
            }
        }

        /* If the character is not a special character, add it to the string */
        if (FALSE == isSpecial)
        {
            pDstStr[outIdx++] = pSrcStr[strIdx];
        }
    }

    /* Add null terminator to the end of the string */
    pDstStr[outIdx] = '\0';
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Get SHA256 of input string
 * @param   pInStr
 * @param   pOutHash
 */
void GetStrSha256(CHAR *pInStr, CHAR *pOutHash)
{
    UINT8       byteCnt;
    UINT8       hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX  sha256;

    /* Init sha256 object */
    SHA256_Init(&sha256);

    /* Update string for hashing */
    SHA256_Update(&sha256, pInStr, strlen(pInStr));

    /* Create final hash */
    SHA256_Final(hash, &sha256);

    /* Convert binary to string data */
    for (byteCnt = 0; byteCnt < SHA256_DIGEST_LENGTH; byteCnt++)
    {
        snprintf(pOutHash + (byteCnt * 2), SHA256_STR_LEN_MAX - (byteCnt * 2), "%02x", hash[byteCnt]);
    }

    /* Terminate with null */
    pOutHash[byteCnt * 2] = '\0';
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Check both IP addresses are in same subnet or not
 * @param   pIpAddr1
 * @param   pIpAddr2
 * @param   pSubnetMask
 * @return  TRUE / FALSE
 */
BOOL IsIpAddrInSameSubnet(CHAR *pIpAddr1, CHAR *pIpAddr2, CHAR *pSubnetMask)
{
    UINT32 nwIp1 = 0, nwIp2 = 0, nwIpSubnet = 0;

    /* Convert IP address 1, IP address 2 and Subnet */
    inet_pton(AF_INET, pIpAddr1, &nwIp1);
    inet_pton(AF_INET, pIpAddr2, &nwIp2);
    inet_pton(AF_INET, pSubnetMask, &nwIpSubnet);

    if ((nwIp1 & nwIpSubnet) == (nwIp2 & nwIpSubnet))
    {
        /* Both IPs are in same subnet */
        return TRUE;
    }

    /* Subnet is mismatched */
    return FALSE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Create non-blocking tcp connection. Generally, it provides EINPROGRESS error in
 *          connect for non-blocking tcp connect
 * @param   pIpAddr
 * @param   port
 * @param   pConnFd
 * @return  TRUE on success; IN_PROGRESS on wait more, FALSE otherwise
 */
BOOL CreateNonBlockConnection(const CHAR *pIpAddr, UINT16 port, INT32 *pConnFd)
{
    NM_IpAddrFamily_e ipAddrFamily = NMIpUtil_GetIpAddrFamily(pIpAddr);

    /* Set remote address information */
    *pConnFd = INVALID_CONNECTION;

    if (ipAddrFamily == NM_IPADDR_FAMILY_V4)
    {
        struct sockaddr_in sockAddr;

        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_port = htons(port);

        /*  Convert remote ip from string to network format */
        inet_pton(AF_INET, pIpAddr, &sockAddr.sin_addr);

        /* Create a non-blocking socket */
        *pConnFd = socket(AF_INET, TCP_NB_SOCK_OPTIONS, 0);
        if (*pConnFd == INVALID_CONNECTION)
        {
            /* Fail to create socket */
            EPRINT(UTILS, "fail to create socket: [ip=%s], [err=%s]", pIpAddr, STR_ERR);
            return FALSE;
        }

        /* Connect the non-blocking socket */
        if (connect(*pConnFd, (struct sockaddr *)&sockAddr, sizeof(sockAddr)))
        {
            /* Wait if connection in progress */
            if (errno == EINPROGRESS)
            {
                /* Wait for connection */
                return IN_PROGRESS;
            }

            /* Close socket on connect failure */
            CloseSocket(pConnFd);
            return FALSE;
        }

        /* Successfully bound socket */
        return TRUE;
    }
    else if (ipAddrFamily == NM_IPADDR_FAMILY_V6)
    {
        struct sockaddr_in6 sockAddr;

        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sin6_family = AF_INET6;
        sockAddr.sin6_port = htons(port);

        /*  Convert remote ip from string to network format */
        inet_pton(AF_INET6, pIpAddr, &sockAddr.sin6_addr);

        /* Create a non-blocking socket */
        *pConnFd = socket(AF_INET6, TCP_NB_SOCK_OPTIONS, 0);
        if (*pConnFd == INVALID_CONNECTION)
        {
            /* Fail to create socket */
            EPRINT(UTILS, "fail to create socket: [ip=%s], [err=%s]", pIpAddr, STR_ERR);
            return FALSE;
        }

        /* Connect the non-blocking socket */
        if (connect(*pConnFd, (struct sockaddr *)&sockAddr, sizeof(sockAddr)))
        {
            /* Wait if connection in progress */
            if (errno == EINPROGRESS)
            {
                /* Wait for connection */
                return IN_PROGRESS;
            }

            /* Close socket on connect failure */
            CloseSocket(pConnFd);
            return FALSE;
        }

        /* Successfully bound socket */
        return TRUE;
    }

    /* Invalid family type */
    return FALSE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Extracts Network Address form ipv4/ipv6 Address
 * @param   pIpString - Ip Address string
 * @param   pSubnetInfo - Subnet String (IPv4)/Prefix Length (IPv6)
 * @param   addrFamily
 * @param   pNwAddr - Network Address
 */

void GetNetworkAddress(CHARPTR pIpString, VOIDPTR pSubnetInfo, UINT8 addrFamily, IP_NW_ADDR_u *pNwAddr)
{
    if (addrFamily == AF_INET)
    {
        CHARPTR         pSubnetMask = (CHARPTR)pSubnetInfo;
        UINT32          nwIpSubnet;
        struct in_addr  addr4;

        /* Convert Ip Address String to Network format */
        inet_pton(AF_INET, pIpString, &addr4);

        /* Convert Subnet String to Network format */
        inet_pton(AF_INET, pSubnetMask, &nwIpSubnet);

        /* Extract & Store Network Address */
        pNwAddr->ip4 = (addr4.s_addr & nwIpSubnet);
    }
    else
    {
        UINT8           prefixLength = *(UINT8PTR)pSubnetInfo;
        UINT8           remainder = (prefixLength % 8);
        struct in6_addr addr6, tAddr6;

        memset(&addr6, 0, sizeof(struct in6_addr));

        /* Convert Ip Address String to Network format */
        inet_pton(AF_INET6, pIpString, &tAddr6);

        /* Store only Network Bits based on prefix length */
        memcpy(&addr6.s6_addr[0], &tAddr6.s6_addr[0], sizeof(UINT8) * (prefixLength / 8));

        /* Store remaining bits if available */
        if (remainder != 0)
        {
            addr6.s6_addr[(prefixLength / 8)] = tAddr6.s6_addr[(prefixLength / 8)] & (UINT8)(0xFF << (8 - remainder));
        }

        /* Store Network Address */
        memcpy(&pNwAddr->ip6, &addr6, sizeof(struct in6_addr));
    }
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Extracts Host Address form ipv4/ipv6 Address
 * @param   pIpString - Ip Address string
 * @param   pSubnetInfo - Subnet String (IPv4)/Prefix Length (IPv6)
 * @param   addrFamily
 * @param   pNwAddr - Network Address
 */

void GetHostAddress(CHARPTR pIpString, VOIDPTR pSubnetInfo, UINT8 addrFamily, IP_NW_ADDR_u *pNwAddr)
{
    if (addrFamily == AF_INET)
    {
        CHARPTR         pSubnetMask = (CHARPTR)pSubnetInfo;
        UINT32          nwIpSubnet;
        struct in_addr  addr4;

        /* Convert Ip Address String to Network format */
        inet_pton(AF_INET, pIpString, &addr4);

        /* Convert Subnet String to Network format */
        inet_pton(AF_INET, pSubnetMask, &nwIpSubnet);

        /* Extract & Store Host Address */
        pNwAddr->ip4 = (addr4.s_addr & (~nwIpSubnet));
    }
    else
    {
        UINT8           prefixLength = *(UINT8PTR)pSubnetInfo;
        UINT8           remainder = (prefixLength % 8);
        struct in6_addr addr6;

        /* Convert Ip Address String to Network format */
        inet_pton(AF_INET6, pIpString, &addr6);

        /* Store only Host Bits based on prefix length */
        memset(&addr6.s6_addr[0], 0, sizeof(UINT8) * (prefixLength / 8));

        /* Mask remaining bits to zero if available */
        if (remainder != 0)
        {
            addr6.s6_addr[(prefixLength / 8)] &= (UINT8)(0xFF >> remainder);
        }

        /* Store Host Address */
        memcpy(&pNwAddr->ip6, &addr6, sizeof(struct in6_addr));
    }
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Convert ipv4 mapped ipv6 socket address to ipv4 socket address if needed
 * @param   sockAddr
 */
void ConvertIpv4MappedIpv6SockAddr(SOCK_ADDR_INFO_u *sockAddr)
{
    /* Is it IPv6 family address? */
    if (sockAddr->sockAddr.sa_family != AF_INET6)
    {
        return;
    }

    /* Is it v4 mapped v6 address? */
    if (FALSE == IN6_IS_ADDR_V4MAPPED(&sockAddr->sockAddr6.sin6_addr))
    {
        return;
    }

    /* Convert v4 mapped v6 address to v4 address */
    SOCK_ADDR_INFO_u localSockAddr;
    memset(&localSockAddr, 0, sizeof(localSockAddr));
    localSockAddr.sockAddr4.sin_family = AF_INET;
    localSockAddr.sockAddr4.sin_addr.s_addr = sockAddr->sockAddr6.sin6_addr.s6_addr32[3];
    localSockAddr.sockAddr4.sin_port = sockAddr->sockAddr6.sin6_port;
    *sockAddr = localSockAddr;
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Get ip address in network format from socket address
 * @param   sockAddr
 * @param   ipAddrNw
 */
void GetIpAddrNwFromSockAddr(const SOCK_ADDR_INFO_u *sockAddr, IP_NW_ADDR_u *ipAddrNw)
{
    /* Reset network address info */
    memset(ipAddrNw, 0, sizeof(IP_NW_ADDR_u));

    /* Check ip address family type */
    if (sockAddr->sockAddr.sa_family == AF_INET6)
    {
        /* Copy ipv6 network address */
        ipAddrNw->ip6 = sockAddr->sockAddr6.sin6_addr;
    }
    else
    {
        /* Copy ipv4 network address */
        ipAddrNw->ip4 = sockAddr->sockAddr4.sin_addr.s_addr;
    }
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Get ip address in string format from socket address
 * @param   sockAddr
 * @param   ipAddrStr
 * @param   addrBufLen
 */
void GetIpAddrStrFromSockAddr(const SOCK_ADDR_INFO_u *sockAddr, CHAR *ipAddrStr, UINT32 addrBufLen)
{
    /* Check ip address family type */
    if (sockAddr->sockAddr.sa_family == AF_INET6)
    {
        /* Convert ipv6 network address to string address */
        inet_ntop(AF_INET6, &sockAddr->sockAddr6.sin6_addr, ipAddrStr, addrBufLen);
    }
    else
    {
        /* Convert ipv4 network address to string address */
        inet_ntop(AF_INET, &sockAddr->sockAddr4.sin_addr, ipAddrStr, addrBufLen);
    }
}

//------------------------------------------------------------------------------------------
/**
 * @brief   Get port in host format from socket address
 * @param   sockAddr
 * @return  host format port
 */
UINT16 GetHostPortFromSockAddr(const SOCK_ADDR_INFO_u *sockAddr)
{
    /* Check ip address family type */
    if (sockAddr->sockAddr.sa_family == AF_INET6)
    {
        /* Return ipv6 host port */
        return htons(sockAddr->sockAddr6.sin6_port);
    }
    else
    {
        /* Return ipv4 host port */
        return htons(sockAddr->sockAddr4.sin_port);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get socket address info for communication
 * @param   ipAddr
 * @param   port
 * @param   sockAddr
 * @return  Returns TRUE on success else returns FALSE
 */
BOOL GetSockAddr(const CHAR *ipAddr, UINT16 port, SOCK_ADDR_INFO_u *sockAddr)
{
    // Validate input parameters
    if ((NULL == ipAddr) || ('\0' == *ipAddr) || (NULL == sockAddr))
    {
        return FAIL;
    }

    /* Reset before set socket info */
    memset(sockAddr, 0, sizeof(SOCK_ADDR_INFO_u));

    /* Convert to Network format with IPv4 */
    if (1 == inet_pton(AF_INET, ipAddr, &sockAddr->sockAddr4.sin_addr))
    {
        /* Set family type and port in network format */
        sockAddr->sockAddr4.sin_family = AF_INET;
        sockAddr->sockAddr4.sin_port = htons(port);
        return SUCCESS;
    }

    /* Convert to Network format with IPv6 */
    if (1 == inet_pton(AF_INET6, ipAddr, &sockAddr->sockAddr6.sin6_addr))
    {
        /* Set family type and port in network format */
        sockAddr->sockAddr6.sin6_family = AF_INET6;
        sockAddr->sockAddr6.sin6_port = htons(port);
        return SUCCESS;
    }

    /* Invalid ip address */
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send multicast message based on given parameters
 * @param   sockFd
 * @param   srcAddr
 * @param   multicastGroupAddr
 * @param   port
 * @param   ifName
 * @param   message
 * @return  status SUCCESS/FAIL
 */
BOOL SendMulticastMessage(INT32 sockFd, CHARPTR srcAddr, CHARPTR multicastAddr, UINT32 port, const CHAR *ifname, CHARPTR message)
{
    SOCK_ADDR_INFO_u    sockAddrInfo;
    INT32               messageLen = strlen(message);

    if (NM_IPADDR_FAMILY_V4 == NMIpUtil_GetIpAddrFamily(srcAddr))
    {
        struct ip_mreq  groupInfo;

        /* Add entry of device address to multicast group address */
        if (inet_pton(AF_INET, multicastAddr, &groupInfo.imr_multiaddr.s_addr) != 1)
        {
            EPRINT(UTILS, "fail to convert multicast ip in nw format: [ip=%s]", multicastAddr);
            return FAIL;
        }

        if (inet_pton(AF_INET, srcAddr, &groupInfo.imr_interface.s_addr) != 1)
        {
            EPRINT(UTILS, "fail to convert lan ip in nw format: [ip=%s]", srcAddr);
            return FAIL;
        }

        if (setsockopt(sockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &groupInfo, sizeof(groupInfo)) != STATUS_OK)
        {
            EPRINT(UTILS, "fail to add ipv4 in multicast group: [err=%s]", STR_ERR);
            return FAIL;
        }

        /* Multicast device discovery message */
        memset(&sockAddrInfo, 0, sizeof(sockAddrInfo));
        sockAddrInfo.sockAddr4.sin_family = AF_INET;
        sockAddrInfo.sockAddr4.sin_addr.s_addr = inet_addr(multicastAddr);
        sockAddrInfo.sockAddr4.sin_port = htons(port);

        if (sendto(sockFd, message, messageLen, MSG_NOSIGNAL, &sockAddrInfo.sockAddr, sizeof(sockAddrInfo.sockAddr4)) != messageLen)
        {
            EPRINT(UTILS, "fail to send ipv4 muticast msg: [dev=%s], [err=%s]", ifname, STR_ERR);
            return FAIL;
        }
    }
    else
    {
        struct ipv6_mreq groupInfo;

        /* Add entry of device address to multicast group address */
        groupInfo.ipv6mr_interface = if_nametoindex(ifname);
        if (inet_pton(AF_INET6, multicastAddr, &groupInfo.ipv6mr_multiaddr) != 1)
        {
            EPRINT(CAMERA_INTERFACE, "fail to convert multicast ip in nw format: [ip=%s]", multicastAddr);
            return FAIL;
        }

        if (setsockopt(sockFd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &groupInfo, sizeof(groupInfo)) != STATUS_OK)
        {
            EPRINT(CAMERA_INTERFACE, "fail to add ipv6 in multicast group:[err=%s]", STR_ERR);
            return FAIL;
        }

        /* Multicast device discovery message */
        memset(&sockAddrInfo, 0, sizeof(sockAddrInfo));
        sockAddrInfo.sockAddr6.sin6_family = AF_INET6;
        inet_pton(AF_INET6, multicastAddr, &sockAddrInfo.sockAddr6.sin6_addr);
        sockAddrInfo.sockAddr6.sin6_port = htons(port);
        sockAddrInfo.sockAddr6.sin6_scope_id = if_nametoindex(ifname);

        if (sendto(sockFd, message, messageLen, MSG_NOSIGNAL, &sockAddrInfo.sockAddr, sizeof(sockAddrInfo.sockAddr6)) != messageLen)
        {
            EPRINT(CAMERA_INTERFACE, "fail to send ipv6 muticast msg: [dev=%s], [err=%s]", ifname, STR_ERR);
            return FAIL;
        }
    }

    return SUCCESS;
}

//----------------------------------------------------------------------------
/**
 * @brief Parse tokenized string values by using delimiter
 * @param dataBuffer
 * @param delimiter
 * @param parseString
 * @param dataLen
 * @param srcString
 * @return Returns SUCCESS/FAIL
 */
BOOL ParseDelimiterValue(CHAR *dataBuffer, CHAR *delimiter, CHAR *parseString, UINT32 dataLen, CHAR **srcString)
{
    /* Get tokenized string */
    CHAR *token = strtok_r(dataBuffer, delimiter, srcString);
    if (token == NULL)
    {
        return FAIL;
    }

    /* Copy tokenized string */
    snprintf(parseString, dataLen, "%s", token);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get IP Address and Port from given http URL
 * @param   urlStr
 * @param   protocol
 * @param   ipAddress
 * @param   port
 * @return  SUSSESS/FAIL
 */
BOOL GetIpAddrAndPortFromUrl(const CHAR *urlStr, const CHAR *protocol, CHAR *ipAddress, UINT16PTR port)
{
    UINT8           strLen = 0;
    CHAR            ipAddrWithPort[DOMAIN_NAME_SIZE_MAX];
    CHAR            urlDupStr[DOMAIN_NAME_SIZE_MAX];
    CHAR            ipAddrStr[IPV6_ADDR_LEN_MAX];
    CHARPTR         restIpAddrStr;
    CHARPTR         pUrlSaveStr = urlDupStr;
    unsigned long   urlPort;

    /* Make a duplicate copy of URL to avoid change in original URL */
    snprintf(urlDupStr, sizeof(urlDupStr), "%s", urlStr);

    /* Get the lenth of protocol prefix */
    strLen = strlen(protocol);

    /* Check if it is in format - Example: For http scheme - http://ipAdderss/RelativePath */
    if (strncmp(pUrlSaveStr, protocol, strLen) != STATUS_OK)
    {
        return FAIL;
    }

    /* Skip protocol string */
    pUrlSaveStr += strLen;

    /* Extract entire ip addr string along with port if given */
    memset(ipAddrWithPort, 0, sizeof(ipAddrWithPort));
    if (FAIL == ParseDelimiterValue(NULL, "/", ipAddrWithPort, sizeof(ipAddrWithPort), &pUrlSaveStr))
    {
        return FAIL;
    }

    /* There is no proper url string */
    if (ipAddrWithPort == NULL)
    {
        return FAIL;
    }

    /* Search for ipv6 address if "[" found */
    if (ipAddrWithPort[0] == '[')
    {
        /* Skip initial "[" used for ipv6 address in url */
        restIpAddrStr = &ipAddrWithPort[1];

        /* Extract Ipv6 address (example : [2001:db8:1::1234]:8080) */
        if (FAIL == ParseDelimiterValue(NULL, "]", ipAddrStr, sizeof(ipAddrStr), &restIpAddrStr))
        {
            return FAIL;
        }

        /* Check if valid ipv6 address found */
        if (NM_IPADDR_FAMILY_V6 != NMIpUtil_GetIpAddrFamily(ipAddrStr))
        {
            return FAIL;
        }

        /* Copy ipv6 address */
        snprintf(ipAddress, IPV6_ADDR_LEN_MAX, "%s", ipAddrStr);

        /* If port specified */
        if (restIpAddrStr[0] != ':')
        {
            /* IPv6 address without port */
            return SUCCESS;
        }

        /* Skip ':' from string */
        restIpAddrStr++;

        /* Reset error number as per strtoul API requirement */
        errno = 0;
        urlPort = strtoul(restIpAddrStr, NULL, 10);
        if ((errno == 0) && (urlPort <= 0xFFFF))
        {
            /* Valid port found in URL */
            *port = urlPort;
        }
    }
    else
    {
        /* IPv4 address URL */
        restIpAddrStr = ipAddrWithPort;

        /* Check if valid ipv4 address found with port (example : 192.168.101.1:8080) */
        if (FAIL == ParseDelimiterValue(NULL, ":", ipAddrStr, sizeof(ipAddrStr), &restIpAddrStr))
        {
            return FAIL;
        }

        /* Check if valid ipv4 address found */
        if (NM_IPADDR_FAMILY_V4 != NMIpUtil_GetIpAddrFamily(ipAddrStr))
        {
            return FAIL;
        }

        /* Copy ipv4 address */
        snprintf(ipAddress, IPV4_ADDR_LEN_MAX, "%s", ipAddrStr);

        /* Store port if specified */
        if (restIpAddrStr[0] == '\0')
        {
            /* IPv4 address without port */
            return SUCCESS;
        }

        /* Reset error number as per strtoul API requirement */
        errno = 0;
        urlPort = strtoul(restIpAddrStr, NULL, 10);
        if ((errno == 0) && (urlPort <= 0xFFFF))
        {
            /* Valid port found in URL */
            *port = urlPort;
        }
    }

    return SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
