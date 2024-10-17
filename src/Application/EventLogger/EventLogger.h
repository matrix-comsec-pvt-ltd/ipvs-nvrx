#if !defined EVENTLOGGER_H
#define EVENTLOGGER_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		EventLogger.h
@brief      This module provides event log and event search APIs.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <time.h>

/* Application Includes */
#include "MxTypedef.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_EVENT_DETAIL_SIZE			(32)
#define MAX_EVENT_ADVANCE_DETAIL_SIZE	(64)

/* Prepare event details for configuration change */
#define GET_EVENT_CONFIG_DETAIL(eventDetail, buffSize, configId)    snprintf(eventDetail, buffSize, "%02d", (INT32)(configId+1));

//#################################################################################################
// @DATA TYPES
//#################################################################################################
// enumerator for event type
typedef enum
{
	LOG_ANY_EVENT = 0,
	LOG_CAMERA_EVENT,
	LOG_SENSOR_EVENT,
	LOG_ALARM_EVENT,
	LOG_SYSTEM_EVENT,
	LOG_STORAGE_EVENT,
	LOG_NETWORK_EVENT,
	LOG_OTHER_EVENT,
	LOG_USER_EVENT,
	LOG_COSEC_EVENT,
	LOG_MAX_EVENT_TYPE
}LOG_EVENT_TYPE_e;

// enumerator for event subtype
typedef enum
{
	// camera event subtypes
	LOG_NO_CAMERA_EVENT = 0,
	LOG_MOTION_DETECTION,
	LOG_VIEW_TEMPERING,
	LOG_CAMERA_SENSOR_1,
	LOG_CAMERA_SENSOR_2,
	LOG_CAMERA_SENSOR_3,
	LOG_CAMERA_ALARM_1,
	LOG_CAMERA_ALARM_2,
	LOG_CAMERA_ALARM_3,
	LOG_CONNECTIVITY,
	LOG_MANUAL_RECORDING,
	LOG_ALARM_RECORDING,
	LOG_SCHEDULE_RECORDING,
	LOG_PRESET_TOUR,
	LOG_SNAPSHOT_SCHEDULE,
	LOG_LINE_CROSS,
	LOG_OBJECT_INTRUTION,
	LOG_AUDIO_EXCEPTION,
	LOG_VIDEO_POP_UP,
	LOG_PRESET_POSITION_CHANGE,
	LOG_MISSING_OBJECT,
	LOG_SUSPICIOUS_OBJECT,
	LOG_LOITERING,
    LOG_CAMERA_STATUS,
	LOG_OBJECT_COUNTING,
    LOG_IMAGE_SETTING,
    LOG_STREAM_PARAM_COPY_TO_CAM,
    LOG_MOTION_DETECTION_COPY_TO_CAM,
    LOG_NO_MOTION_DETECTION,
    LOG_MAX_CAMERA_EVENT,

	// sensor event subtype
	LOG_NO_SENSOR_EVENT = 0,
	LOG_SENSOR_INPUT,
	LOG_MAX_SENSOR_EVENT,

	// alarm event subtype
	LOG_NO_ALARM_EVENT = 0,
	LOG_ALARM_OUTPUT,
	LOG_MAX_ALARM_EVENT,

	// system event subtype
	LOG_NO_SYSTEM_EVENT = 0,
	LOG_POWER_ON,
	LOG_MAINS_EVENT,
	LOG_UNAUTH_IP_ACCESS,
	LOG_RTC_UPDATE,
	LOG_DST_EVENT,
	LOG_SCHEDULE_BACKUP,
	LOG_MANUAL_BACKUP,
	LOG_SYSTEM_RESET,
	LOG_SHUTDOWN,
	LOG_LOGGER_ROLLOVER,
	LOG_RESTART,
	LOG_RECORDING_FAIL,
	LOG_AUTO_CFG_STS_REPORT,
	LOG_TIME_ZONE_UPDATE,
	LOG_TWO_WAY_AUDIO,
    LOG_RECORDING_RESTART,
    LOG_FIRMWARE_UPGRADE,
	LOG_MAX_SYSTEM_EVENT,

	// storage event subtype
	LOG_NO_STORAGE_EVENT = 0,
	LOG_HDD_STATUS,
	LOG_HDD_VOLUME_AT_INIT,
	LOG_HDD_VOLUME,
	LOG_HDD_CLEAN_UP,
	LOG_HDD_VOL_CLEAN_UP,
	LOG_MAX_STORAGE_EVENT,

	// network event subtype
	LOG_NO_NETWORK_EVENT = 0,
	LOG_ETHERNET_LINK,
	LOG_IP_ASSIGN,
	LOG_DDNS_IP_UPDATE,
	LOG_UPLOAD_IMAGE,
	LOG_EMAIL_NOTIFICATION,
	LOG_TCP_NOTIFICATION,
	LOG_SMS_NOTIFICATION,
	LOG_MAC_SERVER_UPDATE,
	LOG_MODEM_STATUS,
    LOG_P2P_STATUS,
    LOG_DHCP_SERVER_IP_ASSIGN,
    LOG_DHCP_SERVER_IP_EXPIRE,
	LOG_MAX_NETWORK_EVENT,

	// other event type
	LOG_NO_OTHER_EVENT = 0,
	LOG_UPGRADE_START,
	LOG_UPGARDE_STATUS_ERASE,	// not present
	LOG_UPGARDE_STATUS_WRITE,	// not present
	LOG_UPGARDE_STATUS_VERIFY,	// not present
	LOG_UPGRADE_RESULT,
	LOG_RESTORE_CONFIG_STRAT,
	LOG_RESTORE_CONFIG_STATUS_ERASE,	// not present
	LOG_RESTORE_CONFIG_STATUS_WRITE,	// not present
	LOG_RESTORE_CONFIG_STATUS_VERIFY,	// not present
	LOG_RESTORE_CONFIG_RESULT,
	LOG_BUZZER_STATUS,
	LOG_USB_STATUS,
	LOG_MANUAL_BACKUP_STATUS,
	LOG_MAX_OTHER_EVENT,

	// user events
	LOG_USER_NO_EVENT = 0,
	LOG_USER_SESSION,
	LOG_MANUAL_TRIGGER,
	LOG_CONFIG_CHANGE,
	LOG_SYS_CONFIGURATION,
	LOG_FIRMWARE,
	LOG_LOGIN_REQUEST,
	LOG_ALLOWED_ACCESS,
	LOG_SESSION_EXPIRE,
    LOG_PASSWORD_RESET,
	LOG_MAX_USER_EVENT,

	// COSEC events
	LOG_COSEC_NO_EVENT = 0,
	LOG_COSEC_RECORDING,
	LOG_COSEC_VIDEO_POP_UP,
	LOG_MAX_COSEC_EVENT

}LOG_EVENT_SUBTYPE_e;

// enumerator for event state
typedef enum
{
	EVENT_NORMAL = 0,
	EVENT_STOP = 0,
	EVENT_FAIL = 0,	
	EVENT_REMOTE = 0,
	EVENT_LOGOUT = 0,
	EVENT_RESTORE = 0,
	EVENT_CREATING = 0,
	EVENT_INCOMPLETE_VOLUME = 0,
	EVENT_REGULAR = 0,
	EVENT_UP = 0,
	EVENT_PPPOE = 0,
	EVENT_DISCONNECT = 0,
	EVENT_CFGDFLT = 0,
	
	EVENT_ACTIVE = 1,
	EVENT_START = 1,
	EVENT_AUTO = 1,
	EVENT_ALERT = 1,
	EVENT_LOGIN = 1,	
	EVENT_NO_DISK = 1,
	EVENT_DEFAULT = 1,
	EVENT_MISSING_DISK = 1,
	EVENT_FORMAT = 1,
	EVENT_BACKUP_FILE = 1,
	EVENT_CHANGE = 1,
	EVENT_DHCP = 1,
	EVENT_UPGRADE = 1,
	EVENT_DOWN = 1,
	EVENT_SUCCESS = 1,
	EVENT_CONNECT = 1,
	EVENT_RINGING = 1,

	EVENT_MANUAL = 2,
	EVENT_LOCAL = 2,
	EVENT_UNKNOWN = 2,
	EVENT_COMPLETE = 2,
	EVENT_MISSING_VOLUME = 2,
	EVENT_FULL = 2,
	EVENT_DEFAULT_FAIL = 2,
	EVENT_READ_ONLY = 2,
    EVENT_SLAAC = 2,

	EVENT_INCOMPLETE = 3,
	EVENT_LOW_MEMORY = 3,
	EVENT_DEFAULT_COMPLETE = 3,
	EVENT_PAUSE = 3,

	EVENT_FAULT = 4,
	EVENT_RESUME= 4,

	EVENT_DISK_BUSY = 5

}LOG_EVENT_STATE_e;

// enumerator to indicate status of read event
typedef enum
{
	EVENT_SEARCH_FAIL = -1,
	EVENT_FOUND,
	NO_MORE_EVENT

}EVENT_SEARCH_RESULT_e;

// event log structure
typedef struct
{
	LOG_EVENT_TYPE_e 		eventType;
	LOG_EVENT_SUBTYPE_e 	eventSubtype;
	LOG_EVENT_STATE_e 		eventState;
	struct tm 				eventTime;
	CHAR 					detail[MAX_EVENT_DETAIL_SIZE];
	CHAR 					advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

}EVENT_LOG_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitEventLogger(void);
//-------------------------------------------------------------------------------------------------
BOOL WriteEvent(LOG_EVENT_TYPE_e eventType, LOG_EVENT_SUBTYPE_e eventSubtype, const CHAR *detail, const CHAR *advncDetail, LOG_EVENT_STATE_e eventState);
//-------------------------------------------------------------------------------------------------
BOOL SendNwEvent(LOG_EVENT_TYPE_e eventType, LOG_EVENT_SUBTYPE_e eventSubtype, CHARPTR detail, LOG_EVENT_STATE_e eventState);
//-------------------------------------------------------------------------------------------------
BOOL StartEventSearch(UINT8 sessionIndex, LOG_EVENT_TYPE_e eventType, struct tm *startTime, struct tm *endTime);
//-------------------------------------------------------------------------------------------------
BOOL StartMoreEventSearch(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
EVENT_SEARCH_RESULT_e ReadEvent(UINT8 sessionIndex, EVENT_LOG_t *eventLogPtr);
//-------------------------------------------------------------------------------------------------
void EndEventSearch(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // EVENTLOGGER_H
