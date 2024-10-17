#ifndef DEBUGLOG_H_
#define DEBUGLOG_H_

#ifdef __cplusplus
extern "C" {
#endif

//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DebugLog.h
@brief      File containing the prototype of different functions for application debug logs
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <errno.h>
#include <netinet/in.h>
#include <syslog.h>

/* Application Includes */
#include "MxTypedef.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define	DEBUG_PRINT_STR_LEN     1024

#define EPRINT(mod, ...)        DebugPrint(LOG_ERR, mod, 'E', (CHARPTR)__func__, __LINE__, __VA_ARGS__)
#define WPRINT(mod, ...)        DebugPrint(LOG_WARNING, mod, 'W', (CHARPTR)__func__, __LINE__, __VA_ARGS__)
#define DPRINT(mod, ...)        DebugPrint(LOG_DEBUG, mod, 'D', (CHARPTR)__func__, __LINE__, __VA_ARGS__)
#define IPRINT(mod, ...)        DebugPrint(LOG_INFO, mod, 'I', (CHARPTR)__func__, __LINE__, __VA_ARGS__)
#define HTML_PAGES_DIR          WEB_DIR_PATH "/html_pages"

//#################################################################################################
// @ENUMERATOR
//#################################################################################################
// Add all supported module with log type in this enum and
// also add corresponding string in ModuleStr.
typedef enum
{
#if !defined(GUI_SYSTEM) && !defined(GUI_SYSTEST)
    SYS_LOG                 = 0,		// System related log
    CAMERA_INTERFACE        = 1,
    CONFIGURATION           = 2,
    DATE_TIME               = 3,
    DDNS_CLIENT             = 4,
    DISK_MANAGER            = 5,
    ETHERNET                = 6,
    EVENT_LOGGER            = 7,
    FTP_CLIENT              = 8,
    HTTP_CLIENT             = 9,
    IMAGE_UPLOAD            = 10,
    INPUT_OUTPUT            = 11,
    LIVE_MEDIA_STREAMER     = 12,
    PLAYBACK_MEDIA          = 13,
    NETWORK_MANAGER         = 14,
    EMAIL_NOTIFY            = 15,
    SMS_NOTIFY              = 16,
    TCP_NOTIFY              = 17,
    PTZ_TOUR                = 18,
    RECORD_MANAGER          = 19,
    RTSP_IFACE              = 20,
    SYSTEM_UPGRADE          = 21,
    EVENT_HANDLER           = 22,
    UTILS                   = 23,
    BACKUP_MANAGER          = 24,
    CAM_EVENT               = 25,
    MAC_CLIENT              = 26,
    ONVIF_CLIENT            = 27,
    P2P_MODULE              = 28,
    DHCP_SERVER             = 29,
    NDD_MANAGER             = 30,
    MP2_TS_PARSER_CLIENT    = 31,
    SNAPSHOT_SCHEDULE       = 32,
    DEVICE_INITIATION       = 33,
    RTSP_CLIENT             = 34,
    CAMERA_INITIATION       = 35,
    PUSH_NOTIFY             = 36,
    NETWORK_LIBRARY         = 37,
    MAX_LOG_LEVEL_MODULE,
#else
    GUI_SYS = 0,
    CONFIG_PAGES,
    APPL_CONTROLLER,
    DEVICE_CLIENT,
    STREAM_REQ,
    GUI_LIVE_MEDIA,
    GUI_PB_MEDIA,
    GUI_SYNC_PB_MEDIA,
    LAYOUT,
    GUI_LIB,
    UTILS,
    MAX_LOG_LEVEL_MODULE,
#endif
}LOG_LEVEL_MODULE;

typedef enum
{
    /* NVR server and addon applications log levels */
    SYSTEM_LOG = 0,
	CAMERA,
	BACKUP_RECORDING_DISK,
	NETWORK_SERVICE,
	STREAM_MEDIA,
	EVENT_ACTION,
	OTHER,
    MAX_SERVER_LEVELS,

    /* GUI Main and GUI Test applications log levels */
    GROUP_1 = 32,
    GROUP_2,
    GROUP_3,
    GROUP_4,
    MAX_GUI_LEVELS

}LOG_LEVEL_e;

typedef	enum
{
    DBG_FILE = 1,
	DBG_SYSLOG,
    DBG_USB,
	MAX_DBG_DESTINATION
}DBG_DESTINATION_e;

typedef enum
{
    SYS_LOG_STS_SUCCESS = 0,
    SYS_LOG_STS_FAIL,
    SYS_LOG_STS_AUTH_ERR,
    SYS_LOG_STS_PARSE_ERR,
    SYS_LOG_STS_RESOURCE_ERR,
    SYS_LOG_STS_MAX
}SYS_LOG_STS_e;

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    DBG_DESTINATION_e   debugDestination;
    INT32               syslogServerPort;
    CHAR                syslogServerAddr[INET6_ADDRSTRLEN];
    BOOL                debugEnable;
    BOOL                remoteLoginEnable;
    UINT64              debugLevels;
}DBG_CONFIG_PARAM_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitDebugLog(void);
//-------------------------------------------------------------------------------------------------
void DebugPrint(UINT8 priority, LOG_LEVEL_MODULE mod, CHAR severity, const CHARPTR func, UINT32 line,
                CHAR const *format, ...) __attribute__((format(printf, 6, 7)));
//-------------------------------------------------------------------------------------------------
void SetUiDebugFlag(DBG_CONFIG_PARAM_t *dbgConfig);
//-------------------------------------------------------------------------------------------------
void ReadDebugConfigFile(void);
//-------------------------------------------------------------------------------------------------
void GetDebugConfig(DBG_CONFIG_PARAM_t* dbgConfig);
//-------------------------------------------------------------------------------------------------
BOOL GetDebugFlag(LOG_LEVEL_MODULE module);
//-------------------------------------------------------------------------------------------------
BOOL SetDebugConfig(DBG_CONFIG_PARAM_t* dbgConfig);
//-------------------------------------------------------------------------------------------------
void RemoveDebugConfigFile(void);
//-------------------------------------------------------------------------------------------------
void UpdateSyslogApp(BOOL status,UINT64 freeSize);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#ifdef __cplusplus
}
#endif
#endif
