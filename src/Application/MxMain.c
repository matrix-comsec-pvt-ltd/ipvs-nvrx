//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		MxMain.c
@brief		File contains the defination of main function which is the entry point of NVR application
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <signal.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/resource.h>

/* Application Includes */
#include "SysTimer.h"
#include "EventLogger.h"
#include "HttpClient.h"
#include "SmtpClient.h"
#include "TcpNotification.h"
#include "SmsNotify.h"
#include "DiskController.h"
#include "CameraInterface.h"
#include "ImageUpload.h"
#include "LiveMediaStreamer.h"
#include "PlaybackMediaStreamer.h"
#include "InstantPlaybackMediaStreamer.h"
#include "NetworkManager.h"
#include "NetworkConfig.h"
#include "NetworkCommand.h"
#include "InputOutput.h"
#include "FtpClient.h"
#include "RecordManager.h"
#include "NetworkController.h"
#include "DebugLog.h"
#include "Utils.h"
#include "DdnsClient.h"
#include "PtzTour.h"
#include "BackupManager.h"
#include "EventHandler.h"
#include "MxOnvifClient.h"
#include "CameraDatabase.h"
#include "Config.h"
#include "MatrixMacClient.h"
#include "MediaStreamer.h"
#include "SyncPlaybackMediaStreamer.h"
#include "SnapshotSchd.h"
#include "RtspClientInterface.h"
#include "TcpClient.h"
#include "CameraInitiation.h"
#include "ClientMediaStreamer.h"
#include "DeviceDefine.h"
#include "CsvParser.h"
#include "P2pInterface.h"
#include "UserSession.h"
#include "FirmwareManagement.h"
#include "DhcpServer.h"
#include "FcmPushNotification.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define DELAY_100MS_IN_NANO_SEC     (TIMER_RESOLUTION_MINIMUM_MSEC * NANO_SEC_PER_MILLI_SEC)
#define WATCHDOG_KICK_TIME_IN_SEC   15

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static pthread_cond_t   deinitSignal = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t  deinitLock = PTHREAD_MUTEX_INITIALIZER;

//#################################################################################################
// @PROTOTYPE
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void SystemRebootCb(INT32 sigNum);
//-------------------------------------------------------------------------------------------------
static void setResourceLimit(void);
//-------------------------------------------------------------------------------------------------
static void checkSystemRebootPendingStatus(void);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Main Entry Level Function of NVR application
 * @param   argc - Number of Passed Command Line Arguments
 * @param   argv - Pointer to buffer of passed argumnets value
 * @return  Exit status of Main
 */
INT32 main(INT32 argc, CHARPTR argv[])
{
	BOOL				watchDogEnabled = TRUE;
	POWER_ACTION_e		powerAct = 0;
    UINT64              tickRefTime;
    UINT64              tickEndTime;
    INT64               tickTimeDiff;
    UINT32              tickElapsedCnt;
    UINT32              watchdogKickCnt = 0;

    /* The First Argument is to enable/disable watchdog */
    if (argc >= 2)
    {
        if (atoi(argv[1]) == (INT32)FALSE)
        {
            watchDogEnabled = FALSE;
        }
    }

    /* register signals */
    /* PARASOFT : Rule OWASP2021-A5-c - No need to check errno */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGUSR2, SystemRebootCb);

    /* Initialize Debug Module First */
    InitDebugLog();

    /* Set application resource limit */
    setResourceLimit();

    /* Check system reboot is pending or not after firmware upgrade */
    checkSystemRebootPendingStatus();

    /* Init system command handler */
    InitSysCmdHandler();

    /* Init device information */
    InitDeviceInfo();

    /* Print device information */
    EPRINT(SYS_LOG, "NVR SERVER APPLICATION: [device=%s], [build=%s], [software=%s], [communication=V%02dR%02d], [board=%s], [multiplier=%s], [extRtc=%s]",
           GetNvrModelStr(), GetBuildDateTimeStr(), GetSoftwareVersionStr(), COMMUNICATION_VERSION, COMMUNICATION_REVISION, GetHardwareBoardVersionStr(),
           IsDiskMultiplierAvailable() ? "Available" : "Not Available", IsExternalRtcAvailable() ? "Available" : "Not Available");

    /* Initialize System Modules */
	InitNetworkConfig();
	InitNetworkCommand();
	InitSysConfig();
	InitEventLogger();
	InitSysTimer();
	InitDateTime();

	WriteEvent(LOG_SYSTEM_EVENT, LOG_POWER_ON, NULL, NULL, EVENT_ALERT);

	InitIO();
	InitUtils();
    InitNetworkController();
	InitDdnsClient();
    InitRtspClientInterface();
	InitHttp();
    InitSmtpClient();
	InitSmsNotification();
	InitTcpNotification();
    InitFcmPushNotification();
	InitFtpClient();
	InitPtzTour();
	InitOnvifClient();
	InitOnvifModelParameter();
	InitCameraInterface();
	InitCameraEventPoll();
	InitImageUpload();
	InitLiveMediaStream();
	InitPlaybackStreamer();
	InitSyncPlaybackStreamer();
	InitInstantPlaybackStreamer();
	InitEventHandler();
	InitMacClient();
    InitUserSession();
	InitNetworkManager();
	InitRecordManager();
	InitBackupManager();
	InitDiskController();
	InitSnapshotSchdUpload();
	InitClientMediaStreamer();
	InitCameraInitiationTcpNw();
	InitTcpClient();
    InitP2pModule();
    InitFirmwareManagement();
    InitDhcpServer();

    /* If updateNotify file is available means configuration have been restored previously
     * or sample file has been updated so update language files accordingly. */
    if (access(LANGUAGE_UPDATE_NOTIFY, F_OK) == STATUS_OK)
	{
        UpdateLanguageFiles();
		remove(LANGUAGE_UPDATE_NOTIFY);
	}

	OnBootEvent(ACTIVE);
    if (TRUE == watchDogEnabled)
    {
        StartWatchDog();
    }

    /* Get current time in nano seconds for reference */
    tickRefTime = GetMonotonicTimeInNanoSec();

    while (TRUE)
	{
        /* Add the nanoseconds of timer tick resolution to derive the Sleep */
        tickRefTime += DELAY_100MS_IN_NANO_SEC;

        /* Get current time in nano seconds */
        tickEndTime = GetMonotonicTimeInNanoSec();

        /* Get the time difference with previous */
        tickTimeDiff = tickRefTime - tickEndTime;
        if (tickTimeDiff > 0)
        {
            /* We have time to wait for next tick */
            SleepNanoSec(tickTimeDiff);
            tickElapsedCnt = 1;
        }
        else
        {
            /* Provide half of base tick */
            SleepNanoSec(DELAY_100MS_IN_NANO_SEC/2);

            /* Convert difference into positive */
            tickTimeDiff = ((-tickTimeDiff)/NANO_SEC_PER_MILLI_SEC);
            if (tickTimeDiff > 0)
            {
                /* Get latest time */
                tickRefTime = GetMonotonicTimeInNanoSec();
            }

            /* Calculate elapsed tick */
            tickElapsedCnt = (tickTimeDiff/TIMER_RESOLUTION_MINIMUM_MSEC)+1;
        }

        /* Provide tick to timer tasks. Compensation logic not added yet */
        RunSysTimer();

        /* Update watchdog ticks */
        watchdogKickCnt += tickElapsedCnt;
        if (watchdogKickCnt >= CONVERT_SEC_TO_TIMER_COUNT(WATCHDOG_KICK_TIME_IN_SEC))
        {
            /* Kick watchdog */
            watchdogKickCnt = 0;
            KickWatchDog();
        }

        powerAct = GetPowerAction();
        if (powerAct != POWER_ACTION_MAX)
		{
			break;
		}
	}

    KickWatchDog();
    if (SUCCESS == KillProcess(GUI_APPL_NAME, SIG_USR))
    {
        MUTEX_LOCK(deinitLock);
        pthread_cond_wait(&deinitSignal, &deinitLock);
        MUTEX_UNLOCK(deinitLock);
    }

	DeinitIO();
    KickWatchDog();

	DeinitClientMediaStreamer();
    KickWatchDog();

	DeinitNwManager();
    KickWatchDog();

	DeInitRecordManager();
    KickWatchDog();

	DeInitBackupManager();
    KickWatchDog();

	DeInitDiskController();
    KickWatchDog();

    DeinitRtspClientInterface();
    KickWatchDog();

    DeInitP2pModule();
    KickWatchDog();

    DeInitFirmwareManagement();
    KickWatchDog();

    DeInitDhcpServer();
    KickWatchDog();

    DeinitFcmPushNotification();
    KickWatchDog();

    DeinitSmtpClient();
    KickWatchDog();

    DeInitNetworkController();
    KickWatchDog();

	sync();

    if (powerAct == REBOOT_DEVICE)
    {
        DPRINT(SYS_LOG, "system is going to restart now...");
        sleep(1);
        reboot(LINUX_REBOOT_CMD_RESTART);
    }
    else
    {
        DPRINT(SYS_LOG, "system is going to power down now...");
        ExeSysCmd(FALSE, "poweroff");
    }

	return EXIT_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Signal handler for system reboot
 * @param   sigNum - Signal number of caused by signal generated
 */
static void SystemRebootCb(INT32 sigNum)
{
    MUTEX_LOCK(deinitLock);
	pthread_cond_signal(&deinitSignal);
    MUTEX_UNLOCK(deinitLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set FD Resource Limit of GUI Application. Bydefault any application can open 1024 FDs
 *          but for rockchip, GUI application required quite more than 1024 FDs. Hence we have to
 *          set this limit during initialization process.
 */
static void setResourceLimit(void)
{
    struct rlimit resourceLimit;

    /* Get Current FD Limit */
    if (getrlimit(RLIMIT_NOFILE, &resourceLimit))
    {
        EPRINT(SYS_LOG, "fail to get max fd limit: [err=%s]", STR_ERR);
        return;
    }

    /* Set Required FDs Count */
    resourceLimit.rlim_cur = 2048;

    /* Set New FD Limit */
    if (setrlimit(RLIMIT_NOFILE, &resourceLimit))
    {
        EPRINT(SYS_LOG, "fail to set max fd limit: [err=%s]", STR_ERR);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Before V8R3, After system firmware upgrade, we apply reboot command from firmware upgrade
 *          application but sometimes system takes time to reboot and it executes updated nvr application.
 *          In new application, if config file version updated then we will migrate from old version to
 *          new version. During config migration, if system rebooted then we may victim of config corruption.
 *          To avoid that, we will write config status file from firmware upgrade application and we will
 *          check same in nvr application. If it is present then we will not start the system and we will
 *          apply force reboot. After V8R3, we applied force reboot in firmware upgrade application.
 */
static void checkSystemRebootPendingStatus(void)
{
    /* Is firmware upgrade confirmation file not present? */
    if (access(FIRMWARE_UPGRADE_CONFIRMATION_FILE, F_OK) != STATUS_OK)
    {
        /* Nothing to do and start system normally */
        return;
    }

    /* Firmware is upgraded and we are waiting for system reboot */
    EPRINT(SYS_LOG, "system is waiting for reboot after firmware upgrade");
    unlink(FIRMWARE_FILE);
    EPRINT(SYS_LOG, "firmware file removed: [file=%s]", FIRMWARE_FILE);
    unlink(FIRMWARE_UPGRADE_CONFIRMATION_FILE);
    EPRINT(SYS_LOG, "firmware upgrade confirmation file removed: [file=%s]", FIRMWARE_UPGRADE_CONFIRMATION_FILE);
    sync();
    EPRINT(SYS_LOG, "system synced and waiting for reboot...");
    sleep(5);
    EPRINT(SYS_LOG, "now apply hard system reboot...");
    sync();
    reboot(LINUX_REBOOT_CMD_RESTART);
}

//#################################################################################################
// END OF FILE
//#################################################################################################
