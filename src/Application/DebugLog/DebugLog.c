//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DebugLog.h
@brief      File containing the API of different functions for application debug logs
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>

/* Application Includes */
#include "DebugLog.h"
#include "CommonApi.h"
#if !defined(GUI_SYSTEM) && !defined(RTSP_CLIENT_APP) && !defined(GUI_SYSTEST) && !defined(SYS_CMD_EXE_APP)
#include "Utils.h"
#endif

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Syslog daemon prints only 250 characters. 26 bytes are used for date time and identity (Sep 23 09:17:29 NVR_APP : ) */
#define SYSLOG_MSG_CHAR_MAX     224
#define DEBUG_CNFG_FILE_VER     3   /* (2 --> 3) IPv6 support added */

// To show file log on web pages we need to create hard link of logs file in html folder
#define LOG_LINK_FILE_FOLDER    HTML_PAGES_DIR "/log"
#define BOOTUP_DMESG_FILE       LOG_LINK_FILE_FOLDER "/dmesg.boot"
#define	DBG_CFG_FILE            CONFIG_DIR_PATH "/debugconfig.cfg"
#ifndef USR_RW_GRP_R_OTH_R
#define USR_RW_GRP_R_OTH_R      (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#endif
#ifndef CREATE_RDWR_SYNC_MODE
#define CREATE_RDWR_SYNC_MODE	(O_CREAT | O_RDWR | O_SYNC | O_CLOEXEC)
#endif
#define MAX_LOG_FILE_SZ			(3 * MEGA_BYTE)
#define MAX_USB_LOG_SIZE        (10 * MEGA_BYTE)
#define LOG_ROTATION            1 // [n+1] files will be rotated
#define LOG_FILE                LOG_DIR_PATH "/log.app"
#define USB_LOG_DIR             MEDIA_DIR_PATH "/Manual_Backup"

#if defined(RK3588_NVRH)
#define REMOTE_SHELL_DAEMON_PATH    "/usr/sbin/sshd"
#else
#define REMOTE_SHELL_DAEMON_PATH    "/usr/sbin/telnetd"
#endif

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    UINT8	version;
    UINT16	size;
}FILE_PARA_t;

typedef struct
{
    DBG_DESTINATION_e		debugDestination;
    INT32					syslogServerPort;
    CHAR					syslogServerAddr[INET_ADDRSTRLEN];
    BOOL					debugEnable;
    BOOL					remoteLoginEnable;
    UINT64					debugLevels;
}DBG_CONFIG_PARAM_VER1_2_t; /* (1 --> 2) Remote login disabled bydefault */

//#################################################################################################
// @GLOBAL VARIABLES
//#################################################################################################
/* Change should also reflect in DebugLog.h */
static const CHARPTR moduleStr[MAX_LOG_LEVEL_MODULE] =
{
#if !defined(GUI_SYSTEM) && !defined(GUI_SYSTEST)
    "SYS_LOG ", //index 0
    "CAM_INFC", //index 1
    "CONFIG  ", //index 2
    "DT_TIME ", //index 3
    "DDNS_CLT", //index 4
    "DISK_MNG", //index 5
    "ETHERNET", //index 6
    "EVT_LOG ", //index 7
    "FTP_CLT ", //index 8
    "HTTP_CLT", //index 9
    "IMG_UPLD", //index 10
    "INP_OUTP", //index 11
    "LV_M_STR", //index 12
    "PLAYBACK", //index 13
    "NET_MNGR", //index 14
    "EML_NTFY", //index 15
    "SMS_NTFY", //index 16
    "TCP_NTFY", //index 17
    "PTZ_TOUR", //index 18
    "REC_MNGR", //index 19
    "RTSP_IFC", //index 20
    "SYS_UPGD", //index 21
    "EVT_HNDL", //index 22
    "UTILS   ", //index 23
    "BKP_MNGR", //index 24
    "CAM_EVNT", //index 25
    "MAC_CLT ", //index 26
    "ONVF_CLT", //index 27
    "P2P_MOD ", //index 28
    "DHCP_SRV", //index 29
    "NDD_MNGR", //index 30
    "MP2_CLT ", //index 31
    "SNAP_SHD", //index 32
    "DEV_INIT", //index 33
    "RTSP_CLT", //index 34
    "CAM_INIT", //index 35
    "PUSH_NTF", //index 36
    "",         //index 37 (for network library module)
#else
    "GUI_SYS ",
    "CNFG_PGS",
    "APP_CTRL",
    "DEV_CLNT",
    "MSG_REQ ",
    "LIVE_MED",
    "ASYNC_PB",
    "SYNC_PB ",
    "LAYOUT  ",
    "GUI_LIB ",
    "UTILS   ",
#endif
};

static DBG_CONFIG_PARAM_t dbgConfigParam =
{
    DBG_FILE,   // debug destination
    514,        // server port
    "",         // server ip
    TRUE,       // debug flag
    FALSE,      // remote login
    1,          // debug levels
};

static const DBG_CONFIG_PARAM_t DfltDbgCnfg =
{
    DBG_FILE,   // debug destination
    514,        // server port
    "",         // server ip
    TRUE,       // debug flag
    FALSE,      // remote login
    1,          // debug levels
};

#if !defined(GUI_SYSTEM) && !defined(RTSP_CLIENT_APP) && !defined(GUI_SYSTEST) && !defined(SYS_CMD_EXE_APP)
static const FILE_PARA_t fileVerPara =
{
    DEBUG_CNFG_FILE_VER,
    sizeof(DBG_CONFIG_PARAM_t),
};
#endif

//Flags indicating Whether debug log is enable or diable
static BOOL debugFlag[MAX_LOG_LEVEL_MODULE];

#if !defined(GUI_SYSTEM) && !defined(RTSP_CLIENT_APP) && !defined(GUI_SYSTEST) && !defined(SYS_CMD_EXE_APP)
static BOOL     usbDiskState = REMOVED;
static UINT64   usbFreeSize = 0;
#endif

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void ExtractDebugFlag(void);
//-------------------------------------------------------------------------------------------------
static void SetClearDebugFlag(UINT64 level, BOOL flagVal);
//-------------------------------------------------------------------------------------------------
static void UpdateDebugConfig(INT32 fileFd, const DBG_CONFIG_PARAM_t *pDebugCnfg);
//-------------------------------------------------------------------------------------------------
#if !defined(GUI_SYSTEM) && !defined(RTSP_CLIENT_APP) && !defined(GUI_SYSTEST) && !defined(SYS_CMD_EXE_APP)
static void WriteDebugConfig(void);
//-------------------------------------------------------------------------------------------------
static void updateRemoteLoginService(BOOL action);
//-------------------------------------------------------------------------------------------------
static void updateSyslogService(BOOL action);
#endif
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initialize debug module and its global variables
 */
void InitDebugLog(void)
{
    /* Initially disable all debugs */
    memset(debugFlag, FALSE, sizeof(debugFlag));

    /* Read debug configuration */
    ReadDebugConfigFile();

#if defined(GUI_SYSTEM)
    /* Open syslog for GUI application */
    openlog("GUI_APP ", LOG_NDELAY, LOG_LOCAL1);
#elif defined(RTSP_CLIENT_APP)
    /* Open syslog for RTSP sub application */
    openlog("RTSP_APP", LOG_NDELAY, LOG_LOCAL2);
#elif defined(GUI_SYSTEST)
    /* Open syslog for GUI test application */
    openlog("GUI_TEST", LOG_NDELAY, LOG_LOCAL3);
#elif defined(SYS_CMD_EXE_APP)
    /* Open syslog for system command exe application */
    openlog("EXE_APP ", LOG_NDELAY, LOG_LOCAL4);
#else
    /* Open syslog for NVR main application */
    openlog("NVR_APP ", LOG_NDELAY, LOG_LOCAL0);

    /* Store bootup dmesg in file and clear */
    ExeSysCmd(TRUE, "dmesg -c > "BOOTUP_DMESG_FILE);

    /* Check and create debug folder if not exist */
    if(access(LOG_DIR_PATH, F_OK) != STATUS_OK)
    {
        mkdir(LOG_DIR_PATH, USR_RWE_GRP_RE_OTH_RE);
    }

    /* Enable remote login service based on configuration */
    updateRemoteLoginService(dbgConfigParam.remoteLoginEnable);

    /* Enable/Disable syslog service based on configuration */
    updateSyslogService(dbgConfigParam.debugEnable);

    /* Check for log link in http root directory if not create soft link */
    if (access(LOG_LINK_FILE_FOLDER, F_OK) != STATUS_OK)
    {
        if (FALSE == ExeSysCmd(TRUE, "ln -s "LOG_DIR_PATH" "HTML_PAGES_DIR))
        {
            /* Failed to create log folder link */
            EPRINT(SYS_LOG, "fail to create log link: [src=%s], [dst%s], [err=%s]", LOG_DIR_PATH, LOG_LINK_FILE_FOLDER, STR_ERR);
        }
    }
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update debug configuration into debug config file for server application only.
 *          Update debug config local copy
 * @param   fileFd - Debug config file FD
 * @param   pDebugCnfg - Debug configuration copy
 */
static void UpdateDebugConfig(INT32 fileFd, const DBG_CONFIG_PARAM_t *pDebugCnfg)
{
#if !defined(GUI_SYSTEM) && !defined(RTSP_CLIENT_APP) && !defined(GUI_SYSTEST) && !defined(SYS_CMD_EXE_APP)
    /* Set pointer at beginning */
    lseek(fileFd, 0, SEEK_SET);

    /* Write file version info */
    if (write(fileFd, &fileVerPara, sizeof(FILE_PARA_t)) != sizeof(FILE_PARA_t))
    {
        return;
    }

    /* Write debug configuration */
    if (write(fileFd, pDebugCnfg, sizeof(DBG_CONFIG_PARAM_t)) != sizeof(DBG_CONFIG_PARAM_t))
    {
        return;
    }

    /* Dump config data */
    fsync(fileFd);
#else
    (void)fileFd;
#endif

    /* Load local debug copy with referred copy. Here source address may same as destination address */
    memmove(&dbgConfigParam, pDebugCnfg, sizeof(DBG_CONFIG_PARAM_t));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Extract module wise debug flags for UINT64 masked variable.
 */
static void ExtractDebugFlag(void)
{
    UINT32 loop;

#if !defined(GUI_SYSTEM) && !defined(GUI_SYSTEST)
    for (loop = 0; loop < MAX_SERVER_LEVELS; loop++)
#else
    for (loop = GROUP_1; loop < MAX_GUI_LEVELS; loop++)
#endif
    {
        SetClearDebugFlag(loop, ((dbgConfigParam.debugLevels >> loop) & 1));
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set/Clear debug 'level' based on value passed to 'flagVal'.
 * @param   level - Debug level
 * @param   flagVal - Debug level value
 */
static void SetClearDebugFlag(UINT64 level, BOOL flagVal)
{
	switch(level)
	{
#if !defined(GUI_SYSTEM) && !defined(GUI_SYSTEST)
        case SYSTEM_LOG:
            debugFlag[SYS_LOG] = flagVal;
            break;

        case CAMERA:
            debugFlag[CAMERA_INTERFACE] = flagVal;
            debugFlag[CAMERA_INITIATION] = flagVal;
            debugFlag[ONVIF_CLIENT] = flagVal;
            debugFlag[PTZ_TOUR] = flagVal;
            break;

        case BACKUP_RECORDING_DISK:
            debugFlag[BACKUP_MANAGER] = flagVal;
            debugFlag[DISK_MANAGER] = flagVal;
            debugFlag[NDD_MANAGER] = flagVal;
            debugFlag[RECORD_MANAGER] = flagVal;
            break;

        case NETWORK_SERVICE:
            debugFlag[DDNS_CLIENT] = flagVal;
            debugFlag[EMAIL_NOTIFY] = flagVal;
            debugFlag[PUSH_NOTIFY] = flagVal;
            debugFlag[ETHERNET] = flagVal;
            debugFlag[FTP_CLIENT] = flagVal;
            debugFlag[HTTP_CLIENT] = flagVal;
            debugFlag[MAC_CLIENT] = flagVal;
            debugFlag[MP2_TS_PARSER_CLIENT] = flagVal;
            debugFlag[NETWORK_MANAGER] = flagVal;
            debugFlag[SMS_NOTIFY] = flagVal;
            debugFlag[TCP_NOTIFY] = flagVal;
            debugFlag[P2P_MODULE] = flagVal;
            debugFlag[DHCP_SERVER] = flagVal;
            debugFlag[NETWORK_LIBRARY] = flagVal;
            break;

        case STREAM_MEDIA:
            debugFlag[RTSP_CLIENT] = flagVal;
            debugFlag[RTSP_IFACE] = flagVal;
            debugFlag[PLAYBACK_MEDIA] = flagVal;
            debugFlag[LIVE_MEDIA_STREAMER] = flagVal;
            break;

        case EVENT_ACTION:
            debugFlag[CAM_EVENT] = flagVal;
            debugFlag[EVENT_LOGGER] = flagVal;
            debugFlag[EVENT_HANDLER] = flagVal;
            debugFlag[IMAGE_UPLOAD] = flagVal;
            debugFlag[SNAPSHOT_SCHEDULE] = flagVal;
            break;

        case OTHER:
            debugFlag[UTILS] = flagVal;
            debugFlag[SYSTEM_UPGRADE] = flagVal;
            debugFlag[INPUT_OUTPUT] = flagVal;
            debugFlag[DATE_TIME] = flagVal;
            debugFlag[CONFIGURATION] = flagVal;
            debugFlag[DEVICE_INITIATION] = flagVal;
            break;
#else
        case GROUP_1:
            debugFlag[LAYOUT] = flagVal;
            debugFlag[APPL_CONTROLLER] = flagVal;
            debugFlag[STREAM_REQ] = flagVal;
            debugFlag[GUI_LIB] = flagVal;
            break;

        case GROUP_2:
            debugFlag[GUI_LIVE_MEDIA] = flagVal;
            break;

        case GROUP_3:
            debugFlag[GUI_PB_MEDIA] = flagVal;
            debugFlag[GUI_SYNC_PB_MEDIA] = flagVal;
            break;

        case GROUP_4:
            debugFlag[DEVICE_CLIENT] = flagVal;
            debugFlag[CONFIG_PAGES] = flagVal;
            debugFlag[UTILS] = flagVal;
            debugFlag[GUI_SYS] = flagVal;
            break;
#endif
        default:
            break;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It is used to prepare debug message and send it to syslog server. It will process only
 *          if error debug or debug module flag enabled. It will add prefix of log level type and
 *          will convert debug in printable format.
 * @param   priority - Syslog log level priority
 * @param   mod - Debug module name
 * @param   severity - Severity tag character
 * @param   func - Called function name
 * @param   line - Called function debug line number
 * @param   format - Pointer to debug fomrat
 */
void DebugPrint(UINT8 priority, LOG_LEVEL_MODULE mod, CHAR severity, const CHARPTR func, UINT32 line, const CHAR *format, ...)
{
    CHAR    debugStr[DEBUG_PRINT_STR_LEN];
    va_list ap;
    INT32   debugLen;
    INT32   prefixLen = 0;

    #if !defined(GUI_SYSTEST)
    /* Nothing to do if debug is disabled, Log module is not valid, Module debug is disabled for non error debug */
    if ((FALSE == dbgConfigParam.debugEnable) || (mod >= MAX_LOG_LEVEL_MODULE) || ((priority != LOG_ERR) && (debugFlag[mod] == FALSE)))
    {
        return;
    }
    #endif

    /* Insert Module Information and Function name & line number */
    if (*moduleStr[mod] != '\0')
    {
        prefixLen = snprintf(debugStr, DEBUG_PRINT_STR_LEN, "%s: %c: %s[%d]: ", moduleStr[mod], severity, func, line);
    }

    /* Append log string */
    va_start(ap,format);
    debugLen = vsnprintf(&debugStr[prefixLen], DEBUG_PRINT_STR_LEN - prefixLen, format, ap);
    va_end(ap);

    /* Validate Length */
    if (debugLen < 0)
    {
        debugStr[prefixLen] = '\0';
        printf("fail to parse debug: [debug=%s]\n", debugStr);
        return;
    }

    /* Syslog daemon will add "\n" in debug message if not added */
#if defined(GUI_SYSTEM)
	syslog(LOG_LOCAL1 | priority, "%s", debugStr);
#elif defined(RTSP_CLIENT_APP)
    syslog(LOG_LOCAL2 | priority, "%s", debugStr);
#elif defined(GUI_SYSTEST)
	syslog(LOG_LOCAL3 | priority, "%s", debugStr);
#elif defined(SYS_CMD_EXE_APP)
    syslog(LOG_LOCAL4 | priority, "%s", debugStr);
#else
    /* Syslog daemon prints only 250 characters (26bytes date-time and module identity overhead of daemon) */
    debugLen += prefixLen;
    if (debugLen > SYSLOG_MSG_CHAR_MAX)
    {
        prefixLen = 0;
        do
        {
            syslog(LOG_LOCAL0 | priority, "%s", &debugStr[prefixLen]);
            prefixLen += SYSLOG_MSG_CHAR_MAX;

        }while(debugLen > prefixLen);
    }
    else
    {
        syslog(LOG_LOCAL0 | priority, "%s", debugStr);
    }
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Read the debug configuration file and update the local copy of debug parameters, start
 *          remote login and syslog if enable. If file not present create one and write default
 *          configuration into it. If version mismatch load default configuration into file.
 */
void ReadDebugConfigFile(void)
{
    INT32       fileFd;
    BOOL        writeDefault = FALSE;
    FILE_PARA_t fileHeader;

    fileFd = open(DBG_CFG_FILE, CREATE_RDWR_SYNC_MODE, USR_RW_GRP_R_OTH_R);
    if (fileFd == INVALID_FILE_FD)
    {
        /* Failed to open debug config file, load default configuration */
        dbgConfigParam = DfltDbgCnfg;

        /* Extract debug flags */
        ExtractDebugFlag();
        return;
    }

    do
    {
        /* Read config header for file version */
        if (read(fileFd, &fileHeader, sizeof(FILE_PARA_t)) != sizeof(FILE_PARA_t))
        {
            /* Failed to read config file header, load default configuration */
            writeDefault = TRUE;
            break;
        }

        /* Is file version invalid? */
        if (fileHeader.version > DEBUG_CNFG_FILE_VER)
        {
            /* Write default configuration */
            writeDefault = TRUE;
            break;
        }

        /* There is no change in config version */
        if (fileHeader.version == DEBUG_CNFG_FILE_VER)
        {
            /* Read configuration from file */
            if (read(fileFd, &dbgConfigParam, sizeof(DBG_CONFIG_PARAM_t)) != sizeof(DBG_CONFIG_PARAM_t))
            {
                /* Failed to read debug config, load default configuration */
                writeDefault = TRUE;
                break;
            }

            /* Config read successfully */
            break;
        }

        DBG_CONFIG_PARAM_VER1_2_t debugConfigV1_2;
        if (fileHeader.version == 1)
        {
            if (read(fileFd, &debugConfigV1_2, sizeof(debugConfigV1_2)) != sizeof(debugConfigV1_2))
            {
                /* Failed to read config file header, load default configuration */
                writeDefault = TRUE;
                break;
            }

            /* Disable remote login bydefault */
            debugConfigV1_2.remoteLoginEnable = FALSE;
            fileHeader.version++;
        }

        if (fileHeader.version == 2)
        {
            if (read(fileFd, &debugConfigV1_2, sizeof(debugConfigV1_2)) != sizeof(debugConfigV1_2))
            {
                /* Failed to read debug config, load default configuration */
                writeDefault = TRUE;
                break;
            }

            /* Convert config from version 2 to 3 */
            dbgConfigParam.remoteLoginEnable = debugConfigV1_2.remoteLoginEnable;
            dbgConfigParam.debugEnable = debugConfigV1_2.debugEnable;
            dbgConfigParam.debugDestination = debugConfigV1_2.debugDestination;
            dbgConfigParam.syslogServerPort = debugConfigV1_2.syslogServerPort;
            snprintf(dbgConfigParam.syslogServerAddr, sizeof(dbgConfigParam.syslogServerAddr), "%s", debugConfigV1_2.syslogServerAddr);
            dbgConfigParam.debugLevels = debugConfigV1_2.debugLevels;
            fileHeader.version++;
        }

        /* Update configuration in file */
        UpdateDebugConfig(fileFd, &dbgConfigParam);

    }while(0);

    /* Need to write default config? */
    if (writeDefault == TRUE)
    {
        /* Write default config */
        UpdateDebugConfig(fileFd, &DfltDbgCnfg);
    }

    /* Close the file */
    close(fileFd);

    /* Extract debug flags */
    ExtractDebugFlag();
}

#if defined(GUI_SYSTEM)
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set the debug configuration parameters into local variables for UI application
 * @param   dbgConfig - Debug configuration parameters
 */
void SetUiDebugFlag(DBG_CONFIG_PARAM_t *dbgConfig)
{
    /* Store new debug info */
    memcpy(&dbgConfigParam, dbgConfig, sizeof(DBG_CONFIG_PARAM_t));
    ExtractDebugFlag();
}

#elif !defined(RTSP_CLIENT_APP) && !defined(GUI_SYSTEST) && !defined(SYS_CMD_EXE_APP)
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write Debug configuration to file.
 */
static void WriteDebugConfig(void)
{
    INT32 fileFd;

    /* Check if file is present or not */
    if (access(DBG_CFG_FILE,F_OK) != 0)
    {
        /* File not present */
        return;
    }

    /* Open the debug config file */
    fileFd = open(DBG_CFG_FILE, CREATE_RDWR_SYNC_MODE, USR_RW_GRP_R_OTH_R);
    if (fileFd == INVALID_FILE_FD)
    {
        syslog(LOG_LOCAL0|LOG_ERR, "fail to open file: [path=%s], [err=%s]", DBG_CFG_FILE, STR_ERR);
        return;
    }

    /* Update debug configuration */
    UpdateDebugConfig(fileFd, &dbgConfigParam);

    /* Close the file */
    close(fileFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write Debug configuration to file.
 * @param   dbgConfig - Debug configuration parameters
 */
void GetDebugConfig(DBG_CONFIG_PARAM_t *dbgConfig)
{
    memcpy(dbgConfig, &dbgConfigParam, sizeof(DBG_CONFIG_PARAM_t));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Debug debug status for log level
 * @param   module - Debug module name
 * @return  Returns debug status (Enable or Disable)
 */
BOOL GetDebugFlag(LOG_LEVEL_MODULE module)
{
    /*PARASOFT : CERT_C-STR31-a - LOG_LEVEL_MODULE defined if neither GUI_SYSTEM or GUI_SYSTEST */
    return debugFlag[module];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start or stop remote login service
 * @param   action - Start or stop
 */
static void updateRemoteLoginService(BOOL action)
{
    switch(action)
    {
        case START:
        {
            if (FALSE == ExeSysCmd(TRUE, "start-stop-daemon -S -b -x " REMOTE_SHELL_DAEMON_PATH))
            {
               EPRINT(SYS_LOG, "fail to start remote login service");
            }
        }
        break;

        case STOP:
        default:
        {
            if (FALSE == ExeSysCmd(TRUE, "start-stop-daemon -K -x " REMOTE_SHELL_DAEMON_PATH))
            {
                 EPRINT(SYS_LOG, "fail to stop remote login service");
            }
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set the debug configuration parameters into local variables and writes into debug config
 *          file. If syslog is selected then standard syslog server will be used for syslog debug.
 *          It also enable/disable remote login as per received flag.
 * @param   dbgConfig - Debug configuration parameters
 * @return  Returns TRUE if stored config successfully else returns FALSE
 */
BOOL SetDebugConfig(DBG_CONFIG_PARAM_t *dbgConfig)
{
    /* Compare debug info changed or not. If not then nothing to do */
    if (memcmp(&dbgConfigParam, dbgConfig, sizeof(DBG_CONFIG_PARAM_t)) == 0)
    {
        /* Debug info not changed */
        return SUCCESS;
    }

    DPRINT(SYS_LOG, "debug config: [debug=%s], [debugDestination=%d], [syslog=%s:%d], [debugLevels=%llx]",
           dbgConfig->debugEnable ? "ENABLE" : "DISABLE", dbgConfig->debugDestination, dbgConfig->syslogServerAddr,
           dbgConfig->syslogServerPort, dbgConfig->debugLevels);

    /* Check remote login status is changed or not. If not then nothing to do */
    if (dbgConfig->remoteLoginEnable != dbgConfigParam.remoteLoginEnable)
    {
        /* Update remote login service */
        updateRemoteLoginService(dbgConfig->remoteLoginEnable);
    }

    /* Store new debug info */
    memcpy(&dbgConfigParam, dbgConfig, sizeof(DBG_CONFIG_PARAM_t));
    WriteDebugConfig();
    ExtractDebugFlag();

    /* Check for syslog */
    updateSyslogService(dbgConfigParam.debugEnable);
	return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove the debug configuration file on reset button action.
 */
void RemoveDebugConfigFile(void)
{
	if(unlink(DBG_CFG_FILE) != STATUS_OK)
	{
        EPRINT(CONFIGURATION, "fail to remove file: [path=%s], [err=%s]", DBG_CFG_FILE,STR_ERR);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update syslog server application (Start or stop)
 * @param   status - Status (Start or Stop)
 * @param   size - Max debug file size
 */
void UpdateSyslogApp(BOOL status, UINT64 size)
{
    usbDiskState = status;
    usbFreeSize = (size*MEGA_BYTE); // convert to Bytes

    DPRINT(SYS_LOG, "usb state change: [status=%d], [freeSize=%llu MB]", usbDiskState, size);
    if ((dbgConfigParam.debugEnable == TRUE) && (dbgConfigParam.debugDestination == DBG_USB))
    {
        updateSyslogService(status);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start or stop syslog service
 * @param   action - Start or stop
 */
static void updateSyslogService(BOOL action)
{
    /* Stop syslogd service if running */
    ExeSysCmd(TRUE, "killall syslogd");

    /* If action stop then nothing to do else start it again */
    if (action != START)
    {
        usbFreeSize = 0;
        return;
    }

    CHAR sysCmd[200];
    switch(dbgConfigParam.debugDestination)
    {
        case DBG_FILE:
        {
            snprintf(sysCmd, sizeof(sysCmd), "syslogd -O %s -s %d -b %d -S", LOG_FILE, (MAX_LOG_FILE_SZ/KILO_BYTE), LOG_ROTATION);
            DPRINT(SYS_LOG, "debug started in file");
        }
        break;

        case DBG_SYSLOG:
        {
            struct in_addr ipv4Addr;

            if (1 == inet_pton(AF_INET, dbgConfigParam.syslogServerAddr, &ipv4Addr))
            {
                snprintf(sysCmd, sizeof(sysCmd), "syslogd -R %s:%d", dbgConfigParam.syslogServerAddr, dbgConfigParam.syslogServerPort);
            }
            else
			{
                snprintf(sysCmd, sizeof(sysCmd), "syslogd -R [%s]:%d", dbgConfigParam.syslogServerAddr, dbgConfigParam.syslogServerPort);
            }

            DPRINT(SYS_LOG, "debug started on syslog");
        }
        break;

        case DBG_USB:
        {
            if ((usbDiskState != ADDED) || (usbFreeSize < MAX_USB_LOG_SIZE))
            {
                return;
            }

            UINT32 logRotation = ((usbFreeSize/MAX_USB_LOG_SIZE)-1);
            logRotation = (logRotation < 99) ? logRotation : 99; // MAX_LOG_ROTATION = 99
            snprintf(sysCmd, sizeof(sysCmd), "syslogd -O %s -s %d -b %d -S", USB_LOG_DIR "/log", (MAX_USB_LOG_SIZE/KILO_BYTE), logRotation);
            DPRINT(SYS_LOG, "debug started in usb");
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        return;
    }

    /* Start syslog client */
    ExeSysCmd(TRUE, sysCmd);
    EPRINT(SYS_LOG, "NVR SERVER APPLICATION: [device=%s], [build=%s], [software=V%02dR%02d.%d], [communication=V%02dR%02d]",
           GetNvrModelStr(), GetBuildDateTimeStr(), SOFTWARE_VERSION, SOFTWARE_REVISION, PRODUCT_SUB_REVISION, COMMUNICATION_VERSION, COMMUNICATION_REVISION);
}
#endif

//#################################################################################################
// END OF FILE
//#################################################################################################
