//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		EventLogger.c
@brief      This module provides event log and event search APIs.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "EventLogger.h"
#include "DateTime.h"
#include "DebugLog.h"
#include "Utils.h"
#include "Config.h"
#include "NetworkManager.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_EVENT_LOG			(10000)
#define EVENT_LOG_VERSION		(2)
#define EVENT_LOG_FILE			EVENT_DIR_PATH "/eventLog.log"
#define EVENT_CNT_FILE			EVENT_DIR_PATH "/eventCnt.log"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
	BOOL 				request;
	BOOL 				eventTypeMatch;
	BOOL 				startTimeMatch;
	BOOL 				endTimeMatch;
	UINT16 				readIndex;
	UINT16 				count;
	LOG_EVENT_TYPE_e 	eventType;
	time_t 				startTimeSec;
	time_t 				endTimeSec;
	pthread_mutex_t 	sessionMutex;
}SEARCH_SESSION_t;

typedef struct
{
	UINT32 				type : 8;
	UINT32 				subtype : 8;
	UINT32 				state : 8;
	UINT32 				resv : 8;
	time_t 				timeSec;
	CHAR	 			detail[MAX_EVENT_DETAIL_SIZE];
	CHAR	 			advncdetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];
}FILE_EVENT_LOG_t;

typedef struct
{
	UINT32 				version : 8;
	UINT32 				rolloverFlag : 8;
	UINT32 				writeIndex : 16;
}EVENT_CNT_t;

typedef struct
{
	UINT32 				fileWrite;
	UINT32 				nwWrite;
}EVENT_LOG_STATE_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL getEvtSearchSessState(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static void checkEventWriteStatus(LOG_EVENT_TYPE_e eventType, LOG_EVENT_SUBTYPE_e eventSubtype, UINT8PTR fileWr, UINT8PTR nwWr);
//-------------------------------------------------------------------------------------------------
static void printEventInfo(EVENT_CNT_t *eventCnt, EVENT_LOG_t *evPtr, BOOL isLiveEvt, BOOL isFileWrite);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static EVENT_CNT_t			eventCnt;
static SEARCH_SESSION_t 	searchSession[MAX_NW_CLIENT];
static pthread_mutex_t 		sessionListMutex;
static pthread_mutex_t 		eventWriteMutex;

static const CHARPTR eventLogTypeStr[LOG_MAX_EVENT_TYPE] =
{
    "NO EVENT",
    "CAMERA EVENT",
    "SENSOR EVENT",
    "ALARM EVENT",
    "SYSTEM_EVENT",
	"STORAGE_EVENT",
	"NETWORK_EVENT",
    "OTHER EVENT",
    "USER_EVENT",
    "COSEC_EVENT"
};

static const CHARPTR cameraEventSubTypeStr[LOG_MAX_CAMERA_EVENT] =
{
    "CAMERA_EVENT_NO",
    "MOTION_DETECTION",
    "VIEW_TEMPERING",
    "CAMERA_SENSOR_1",
    "CAMERA_SENSOR_2",
    "CAMERA_SENSOR_3",
    "CAMERA_ALARM_1",
    "CAMERA_ALARM_2",
    "CAMERA_ALARM_3",
    "CONNECTIVITY",
    "MANUAL_RECORDING",
    "ALARM_RECORDING",
    "SCHEDULE_RECORDING",
    "PRESET_TOUR",
    "SNAPSHOT_SCHEDULE",
    "TRIP_WIRE",
    "OBJECT_INTRUTION",
    "AUDIO_EXCEPTION",
    "VIDEO_POP_UP",
	"PRESET_POSITION_CHANGE",
    "MISSING_OBJECT",
    "SUSPICIOUS_OBJECT",
    "LOITERING",
    "CAMERA_STATUS",
    "OBJECT_COUNTING",
    "IMAGE_SETTINGS",
    "STREAM_PARAM_COPY_TO_CAM",
    "MOTION_DETECTION_COPY_TO_CAM",
    "NO_MOTION_DETECTION"
};

static const CHARPTR sensorEventSubTypeStr[LOG_MAX_SENSOR_EVENT] =
{
    "NO_SENSOR_EVENT",
    "SENSOR_INPUT"
};

static const CHARPTR alarmEventSubTypeStr[LOG_MAX_ALARM_EVENT] =
{
    "NO_ALARM_EVENT",
    "ALARM_OUTPUT"
};

static const CHARPTR systemEventSubTypeStr[LOG_MAX_SYSTEM_EVENT] =
{
    "NO_SYSTEM_EVENT",
    "POWER_ON",
    "MAINS_EVENT",
    "UNAUTH_IP_ACCESS",
    "RTC_UPDATE",
    "DST_EVENT",
    "SCHEDULE_BACKUP",
    "MANUAL_BACKUP",
    "SYSTEM_RESET",
    "SHUTDOWN",
    "ROLLOVER",
    "SYSTEM_REBOOT",
    "RECORDING_FAIL",
    "AUTO_CFG_STS_REPORT",
    "TIME_ZONE_UPDATE",
    "TWO_WAY_AUDIO",
    "RECORDING_RESTART",
    "FIRMWARE_UPGRADE",
};

static const CHARPTR storageEventSubTypeStr[LOG_MAX_STORAGE_EVENT] =
{
    "NO_STORAGE_EVENT",
    "HDD_STATUS",
    "HDD_VOLUME_AT_INIT",
    "HDD_VOLUME",
    "HDD_CLEAN_UP",
    "HDD_VOLUME_CLEANUP"
};

static const CHARPTR networkeEventSubTypeStr[LOG_MAX_NETWORK_EVENT] =
{
    "NO_NETWORK_EVENT",
    "ETHERNET_LINK",
    "IP_ASSIGN",
    "DDNS_IP_UPDATE",
    "UPLOAD_IMAGE",
    "EMAIL_NOTIFICATION",
    "TCP_NOTIFICATION",
    "SMS_NOTIFICATION",
    "MAC_SERVER_UPDATED",
    "MODEM_STATUS",
    "P2P_STATUS",
    "DHCP_SERVER_IP_ASSIGN",
    "DHCP_SERVER_IP_EXPIRE",
};

static const CHARPTR otherEventSubTypeStr[LOG_MAX_OTHER_EVENT] =
{
    "NO_OTHER_EVENT",
    "UPGRADE_START",
    "UPGRADE_ERASE",
    "UPGARDE_WRITE",
    "UPGARDE_VERIFY",
    "UPGRADE_RESULT",
    "RESTORE_STRAT",
    "RESTORE_ERASE",
    "RESTORE_WRITE",
    "RESTORE_VERIFY",
    "RESTORE_RESULT",
    "BUZZER_STATUS",
    "USB_STATUS",
    "MANUAL_BACKUP"
};

static const CHARPTR userEventSubTypeStr[LOG_MAX_USER_EVENT] =
{
    "USER_NO_EVENT",
    "USER_SESSION",
    "MANUAL_TRIGGER",
    "CONFIG_CHANGE",
    "SYS_CONFIGURATION",
    "FIRMWARE",
    "LOGIN_REQUEST",
    "ALLOWED_ACCESS",
    "SESSION_EXPIRE",
    "PASSWORD_RESET"
};

static const CHARPTR cosecEventSubTypeStr[LOG_MAX_COSEC_EVENT] =
{
    "COSEC_NO_EVENT",
    "COSEC_RECORDING",
    "VIDEO_POP_UP"
};

/** @note:  This is bit wise status */
static const EVENT_LOG_STATE_t eventLogStatus[LOG_MAX_EVENT_TYPE] =
{
    /* {file write, network write} */
    {          0,           0},	// No Event
    { 0x1F73FFFE,  0x117FFFFE},	// Camera Event
    {     0x0002,      0x0002},	// Sensor Event
    {     0x0002,      0x0002},	// Alarm Event
    {    0x30FFA,     0x3FBF8},	// System Event
    {     0x003E,      0x002A},	// Storage Event
    {     0x1FFE,      0x00F2},	// Network Event
    {     0x0462,      0x3FFE},	// Other Event
    {     0x027E,      0x019C},	// User Event
    {     0x0002,      0x0006},	// Cosec Event
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This module initializes the event log module
 */
void InitEventLogger(void)
{
    FILE    *pFileFp;
    UINT8   sessionIndex;

	eventCnt.version = EVENT_LOG_VERSION;
	eventCnt.rolloverFlag = CLEAR;
	eventCnt.writeIndex = 0;

    if(access(EVENT_DIR_PATH, F_OK) != STATUS_OK)
	{
        mkdir(EVENT_DIR_PATH, USR_RWE_GRP_RE_OTH_RE);
    }

	if(access(EVENT_CNT_FILE, F_OK) != STATUS_OK)
	{
        EPRINT(EVENT_LOGGER, "event file not present");
        pFileFp = fopen(EVENT_CNT_FILE, "w");
        if(pFileFp != NULL)
		{
            if(fwrite(&eventCnt, sizeof(EVENT_CNT_t), 1, pFileFp) != 1)
			{
                EPRINT(EVENT_LOGGER, "fail to write event file: [err=%s]", STR_ERR);
			}
            fclose(pFileFp);
			unlink(EVENT_LOG_FILE);
		}
		else
		{
            EPRINT(EVENT_LOGGER, "fail to open event file: [err=%s]", STR_ERR);
		}
	}
	else
	{
        /* Open file for reading */
        pFileFp = fopen(EVENT_CNT_FILE, "r");
        if(pFileFp != NULL)
		{
            if(fread(&eventCnt, (sizeof(EVENT_CNT_t)), 1, pFileFp) != 1)
			{
                EPRINT(EVENT_LOGGER, "fail to read event file: [err=%s]", STR_ERR);
			}

            /* Close current file */
            fclose(pFileFp);

            /* Check file version and if it mismatched then create new one */
			if(eventCnt.version != EVENT_LOG_VERSION)
			{
                /* Create empty fresh file */
                pFileFp = fopen(EVENT_CNT_FILE, "w");
                if(pFileFp != NULL)
                {
                    eventCnt.version = EVENT_LOG_VERSION;
                    eventCnt.rolloverFlag = CLEAR;
                    eventCnt.writeIndex = 0;

                    if(fwrite(&eventCnt, (sizeof(EVENT_CNT_t)), 1, pFileFp) != 1)
                    {
                        EPRINT(EVENT_LOGGER, "fail to write event file: [err=%s]", STR_ERR);
                    }

                    /* Close the file and remove older log file */
                    fclose(pFileFp);
                    unlink(EVENT_LOG_FILE);
                }
			}

            DPRINT(EVENT_LOGGER, "event file info: [version=%d], [rollover=%d], [writeIndex=%d]",
                   eventCnt.version, eventCnt.rolloverFlag, eventCnt.writeIndex);
		}
		else
		{
            EPRINT(EVENT_LOGGER, "fail to read event file: [err=%s]", STR_ERR);
		}
	}

    MUTEX_INIT(sessionListMutex, NULL);
    MUTEX_INIT(eventWriteMutex, NULL);

	for(sessionIndex = 0; sessionIndex < MAX_NW_CLIENT; sessionIndex++)
	{
		searchSession[sessionIndex].request	= FREE;
		searchSession[sessionIndex].eventType = LOG_ANY_EVENT;
		searchSession[sessionIndex].startTimeSec = 0;
		searchSession[sessionIndex].endTimeSec = 0;
		searchSession[sessionIndex].eventTypeMatch = CLEAR;
		searchSession[sessionIndex].startTimeMatch = CLEAR;
		searchSession[sessionIndex].endTimeMatch = CLEAR;
		searchSession[sessionIndex].readIndex = 0;
		searchSession[sessionIndex].count = 0;
        MUTEX_INIT(searchSession[sessionIndex].sessionMutex, NULL);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API write the Event to file.
 * @param   eventType
 * @param   eventSubtype
 * @param   detail
 * @param   advncDetail
 * @param   eventState
 * @return
 */
BOOL WriteEvent(LOG_EVENT_TYPE_e eventType, LOG_EVENT_SUBTYPE_e eventSubtype, const CHAR *detail, const CHAR *advncDetail, LOG_EVENT_STATE_e eventState)
{
    UINT8				fileWrite, isLiveEvtOnNw;
    UINT8               sessionIndex;
	FILE				*fdCnt, *fdEvt;
    EVENT_CNT_t			tempEvtCnt = { 0 };
    EVENT_LOG_t 		eventLog = { 0 };
	FILE_EVENT_LOG_t 	writeEventLog = { 0 };

	writeEventLog.type = eventType;
	writeEventLog.subtype = eventSubtype;
	writeEventLog.state = eventState;
	if(detail != NULL)
	{
       snprintf(writeEventLog.detail, MAX_EVENT_DETAIL_SIZE, "%s", detail);
    }

	if(advncDetail != NULL)
	{
        snprintf(writeEventLog.advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", advncDetail);
    }

	if(SUCCESS != GetLocalTimeInSec(&writeEventLog.timeSec))
	{
        EPRINT(EVENT_LOGGER, "fail to get local time");
	}

    checkEventWriteStatus(eventType, eventSubtype, &fileWrite, &isLiveEvtOnNw);

	if(fileWrite == TRUE)
	{
        MUTEX_LOCK(eventWriteMutex);
		do
		{
			fdEvt = fopen(EVENT_LOG_FILE, "r+");
			if(fdEvt == NULL)
			{
				fdEvt = fopen(EVENT_LOG_FILE, "w");
				if(fdEvt == NULL)
				{
                    EPRINT(EVENT_LOGGER, "fail to open event file: [file=%s], [err=%s]", EVENT_LOG_FILE, STR_ERR);
					fileWrite = FAIL;
					break;
				}
				eventCnt.writeIndex = 0;
				eventCnt.rolloverFlag = CLEAR;
			}

            fseek(fdEvt, (eventCnt.writeIndex * (sizeof(FILE_EVENT_LOG_t))), SEEK_SET);
			if(fwrite(&writeEventLog, sizeof(FILE_EVENT_LOG_t), 1, fdEvt) != 1)
			{
				fclose(fdEvt);
                EPRINT(EVENT_LOGGER, "fail to write event file: [file=%s], [err=%s]", EVENT_LOG_FILE, STR_ERR);
				fileWrite = FAIL;
				break;
			}

			fflush(fdEvt);
			fclose(fdEvt);

			memcpy(&tempEvtCnt, &eventCnt, sizeof(EVENT_CNT_t));
			eventCnt.writeIndex++;
			if(eventCnt.writeIndex >= MAX_EVENT_LOG)
			{
                /* Write index reached to max event log count. Hence set to rollover */
				eventCnt.writeIndex = 0;
				eventCnt.rolloverFlag = SET;
			}

			fdCnt = fopen(EVENT_CNT_FILE, "r+");
			if(fdCnt == NULL)			//File is not created
			{
                EPRINT(EVENT_LOGGER, "fail to open event cnt file: [file=%s], [err=%s]", EVENT_CNT_FILE, STR_ERR);
				fileWrite = FAIL;
				break;
			}

			if(fwrite(&eventCnt, sizeof(EVENT_CNT_t), 1, fdCnt) != 1)
			{
                EPRINT(EVENT_LOGGER, "fail to write event cnt file: [file=%s], [err=%s]", EVENT_CNT_FILE, STR_ERR);
				fileWrite = FAIL;
			}
			else
			{
				fflush(fdCnt);
			}
			fclose(fdCnt);
		}
		while(0);
        MUTEX_UNLOCK(eventWriteMutex);

		for(sessionIndex = 0; sessionIndex < MAX_NW_CLIENT; sessionIndex++)
		{
            if (FREE == getEvtSearchSessState(sessionIndex))
			{
                continue;
            }

            MUTEX_LOCK(searchSession[sessionIndex].sessionMutex);
            if ((tempEvtCnt.rolloverFlag == SET) && (searchSession[sessionIndex].readIndex == tempEvtCnt.writeIndex))
            {
                searchSession[sessionIndex].readIndex++;
                if(searchSession[sessionIndex].readIndex >= MAX_EVENT_LOG)
                {
                    searchSession[sessionIndex].readIndex = 0;
                }
            }
            else
            {
                if(searchSession[sessionIndex].count < MAX_EVENT_LOG)
                {
                    searchSession[sessionIndex].count++;
                }
            }
            MUTEX_UNLOCK(searchSession[sessionIndex].sessionMutex);
		}
	}

	eventLog.eventType = writeEventLog.type;
	eventLog.eventSubtype = writeEventLog.subtype;
    snprintf(eventLog.detail, MAX_EVENT_DETAIL_SIZE, "%s", writeEventLog.detail);
	eventLog.eventState = writeEventLog.state;
    snprintf(eventLog.advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", writeEventLog.advncdetail);
	
	if(SUCCESS != ConvertLocalTimeInBrokenTm(&writeEventLog.timeSec, &eventLog.eventTime))
	{
        EPRINT(EVENT_LOGGER, "fail to get local time in broken");
	}

    if (isLiveEvtOnNw == TRUE)
	{
		if((eventType == LOG_USER_EVENT) && (eventSubtype == LOG_CONFIG_CHANGE))
		{
			// Do not send live event of configuration change
			// other then table id is 17 (Camera Config) & table id 51 (network devices) & table id 1 (general config)
            if ((detail != NULL) && ((strcmp(detail, "01") == STATUS_OK) || (strcmp(detail, "17") == STATUS_OK) || (strcmp(detail, "51") == STATUS_OK)))
			{
				SendEvent(&eventLog);
			}
		}
		else
		{
			SendEvent(&eventLog);
		}
	}

    printEventInfo(&tempEvtCnt, &eventLog, isLiveEvtOnNw, fileWrite);
	return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API sends event to network only.
 * @param   eventType
 * @param   eventSubtype
 * @param   detail
 * @param   eventState
 * @return
 */
BOOL SendNwEvent(LOG_EVENT_TYPE_e eventType, LOG_EVENT_SUBTYPE_e eventSubtype, CHARPTR detail, LOG_EVENT_STATE_e eventState)
{
    EVENT_LOG_t eventLog = {
		.eventType 			= eventType,
		.eventSubtype 		= eventSubtype,
		.eventState 		= eventState,
		.eventTime			= { 0 },
		.detail				= { '\0' },
		.advncDetail		= { '\0' }
	};
	
	if(detail != NULL)
	{
		snprintf(eventLog.detail, MAX_EVENT_DETAIL_SIZE, "%s", detail);
	}
	
	GetLocalTimeInBrokenTm(&eventLog.eventTime);
    SendEvent(&eventLog);
	return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API stores search parameter and starts search session
 * @param   sessionIndex
 * @param   eventType
 * @param   startTime
 * @param   endTime
 * @return
 */
BOOL StartEventSearch(UINT8 sessionIndex, LOG_EVENT_TYPE_e eventType, struct tm *startTime, struct tm *endTime)
{
    EVENT_CNT_t tempEvtCnt;

    if (sessionIndex >= MAX_NW_CLIENT)
	{
        return FAIL;
    }

    MUTEX_LOCK(sessionListMutex);
    searchSession[sessionIndex].request = BUSY;
    MUTEX_UNLOCK(sessionListMutex);

    searchSession[sessionIndex].eventType = eventType;
    ConvertLocalTimeInSec(startTime, &searchSession[sessionIndex].startTimeSec);
    ConvertLocalTimeInSec(endTime, &searchSession[sessionIndex].endTimeSec);

    MUTEX_LOCK(eventWriteMutex);
    memcpy(&tempEvtCnt, &eventCnt, sizeof(EVENT_CNT_t));
    MUTEX_UNLOCK(eventWriteMutex);

    /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
    MUTEX_LOCK(searchSession[sessionIndex].sessionMutex);

    /* If rollover set the set max event log else set write index */
    searchSession[sessionIndex].count = (tempEvtCnt.rolloverFlag == CLEAR) ? tempEvtCnt.writeIndex : MAX_EVENT_LOG;

    /* Start reading from last write index */
    searchSession[sessionIndex].readIndex = tempEvtCnt.writeIndex-1;
    MUTEX_UNLOCK(searchSession[sessionIndex].sessionMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start event search for more events
 * @param   sessionIndex
 * @return
 */
BOOL StartMoreEventSearch(UINT8 sessionIndex)
{
    if (sessionIndex >= MAX_NW_CLIENT)
    {
        return FAIL;
    }

    MUTEX_LOCK(sessionListMutex);
    searchSession[sessionIndex].request = BUSY;
    MUTEX_UNLOCK(sessionListMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API searches for the matching event in the file and outputs to user
 * @param   sessionIndex
 * @param   eventLogPtr
 * @return
 */
EVENT_SEARCH_RESULT_e ReadEvent(UINT8 sessionIndex, EVENT_LOG_t *eventLogPtr)
{
    FILE                *fdEvt;
    FILE_EVENT_LOG_t    readEventLog;

    if((sessionIndex >= MAX_NW_CLIENT) || (eventLogPtr == NULL))
	{
        return NO_MORE_EVENT;
    }

    fdEvt = fopen(EVENT_LOG_FILE, "r");
    if(fdEvt == NULL)
    {
        EPRINT(EVENT_LOGGER, "fail to open event file: [file=%s], [err=%s]", EVENT_LOG_FILE, STR_ERR);
        return EVENT_SEARCH_FAIL;
    }

    MUTEX_LOCK(searchSession[sessionIndex].sessionMutex);
    if(searchSession[sessionIndex].readIndex > 0)
    {
        // Do fseek first time and then read
        fseek(fdEvt, (searchSession[sessionIndex].readIndex * (sizeof(FILE_EVENT_LOG_t))), SEEK_SET);
    }

    while(searchSession[sessionIndex].count > 0)
    {
        if(fread(&readEventLog, sizeof(FILE_EVENT_LOG_t), 1, fdEvt) != 1)
        {
            MUTEX_UNLOCK(searchSession[sessionIndex].sessionMutex);
            EPRINT(EVENT_LOGGER, "fail to read event log: [err=%s]", STR_ERR);
            fclose(fdEvt);
            return EVENT_SEARCH_FAIL;
        }

        searchSession[sessionIndex].count--;
        searchSession[sessionIndex].readIndex = (searchSession[sessionIndex].readIndex > 0) ? (searchSession[sessionIndex].readIndex-1) : (MAX_EVENT_LOG-1);
        searchSession[sessionIndex].eventTypeMatch = (searchSession[sessionIndex].eventType == LOG_ANY_EVENT) ? SET : CLEAR;
        searchSession[sessionIndex].startTimeMatch = (searchSession[sessionIndex].startTimeSec == 0) ? SET : CLEAR;
        searchSession[sessionIndex].endTimeMatch = (searchSession[sessionIndex].endTimeSec == 0) ? SET : CLEAR;

        // IF EVENT TYPE MATCH IS NOT SET AND READ EVENT TYPE MATCHES WITH SEARCH EVENT TYPE
        if((searchSession[sessionIndex].eventTypeMatch == CLEAR) && (readEventLog.type == searchSession[sessionIndex].eventType))
        {
            searchSession[sessionIndex].eventTypeMatch = SET;
        }

        // IF EVENT START TIME MATCH IS NOT SET AND READ EVENT TIME IS GREATER THAN SEARCH START TIME
        if((searchSession[sessionIndex].startTimeMatch == CLEAR) && (readEventLog.timeSec >= searchSession[sessionIndex].startTimeSec))
        {
            searchSession[sessionIndex].startTimeMatch = SET;
        }

        // IF EVENT END TIME MATCH IS NOT SET AND READ EVENT TIME IS LESSER THAN SEARCH TIME MATCH
        if((searchSession[sessionIndex].endTimeMatch == CLEAR) && (readEventLog.timeSec <= searchSession[sessionIndex].endTimeSec))
        {
            searchSession[sessionIndex].endTimeMatch = SET;
        }

        // IF EVENT TYPE MATCH IS SET AND EVENT START TIME IS SET AND EVENT END TIME IS SET
        if((searchSession[sessionIndex].eventTypeMatch == SET)
                && (searchSession[sessionIndex].startTimeMatch == SET) && (searchSession[sessionIndex].endTimeMatch == SET))
        {
            eventLogPtr->eventType = readEventLog.type;
            eventLogPtr->eventSubtype = readEventLog.subtype;
            eventLogPtr->eventState	= readEventLog.state;
            ConvertLocalTimeInBrokenTm(&readEventLog.timeSec, &eventLogPtr->eventTime);
            snprintf(eventLogPtr->detail, MAX_EVENT_DETAIL_SIZE, "%s", readEventLog.detail);
            snprintf(eventLogPtr->advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", readEventLog.advncdetail);
            MUTEX_UNLOCK(searchSession[sessionIndex].sessionMutex);
            fclose(fdEvt);
            return EVENT_FOUND;
        }

        fseek(fdEvt, (searchSession[sessionIndex].readIndex * (sizeof(FILE_EVENT_LOG_t))), SEEK_SET);
    }
    fclose(fdEvt);
    MUTEX_UNLOCK(searchSession[sessionIndex].sessionMutex);
    return NO_MORE_EVENT;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API ends the search session
 * @param   sessionIndex
 */
void EndEventSearch(UINT8 sessionIndex)
{
    if(sessionIndex >= MAX_NW_CLIENT)
    {
        return;
    }

    MUTEX_LOCK(sessionListMutex);
    searchSession[sessionIndex].request = FREE;
    MUTEX_UNLOCK(sessionListMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives search session status
 * @param   sessionIndex
 * @return
 */
static BOOL getEvtSearchSessState(UINT8 sessionIndex)
{
    if (sessionIndex >= MAX_NW_CLIENT)
	{
        return FREE;
    }

    MUTEX_LOCK(sessionListMutex);
    BOOL retVal = searchSession[sessionIndex].request;
    MUTEX_UNLOCK(sessionListMutex);
	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function whether the event should given to network manager or not.
 * @param   eventType
 * @param   eventSubtype
 * @param   fileWr
 * @param   nwWr
 */
static void checkEventWriteStatus(LOG_EVENT_TYPE_e eventType, LOG_EVENT_SUBTYPE_e eventSubtype, UINT8PTR fileWr, UINT8PTR nwWr)
{
    *fileWr = (UINT8)((UINT16)(eventLogStatus[eventType].fileWrite >> eventSubtype) & (UINT16)0x0001);
    *nwWr = (UINT8)((UINT16)(eventLogStatus[eventType].nwWrite >> eventSubtype) & (UINT16)0x0001);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function prints the event information.
 * @param   eventCnt
 * @param   evPtr
 * @param   isLiveEvt
 * @param   isFileWrite
 */
static void printEventInfo(EVENT_CNT_t *eventCnt, EVENT_LOG_t *evPtr, BOOL isLiveEvt, BOOL isFileWrite)
{
    CHARPTR eventSubString;

	switch(evPtr->eventType)
	{
        case LOG_CAMERA_EVENT:
            eventSubString = cameraEventSubTypeStr[evPtr->eventSubtype];
            break;

        case LOG_SENSOR_EVENT:
            eventSubString = sensorEventSubTypeStr[evPtr->eventSubtype];
            break;

        case LOG_ALARM_EVENT:
            eventSubString = alarmEventSubTypeStr[evPtr->eventSubtype];
            break;

        case LOG_SYSTEM_EVENT:
            eventSubString = systemEventSubTypeStr[evPtr->eventSubtype];
            break;

        case LOG_STORAGE_EVENT:
            eventSubString = storageEventSubTypeStr[evPtr->eventSubtype];
            break;

        case LOG_NETWORK_EVENT:
            eventSubString = networkeEventSubTypeStr[evPtr->eventSubtype];
            break;

        case LOG_OTHER_EVENT:
            eventSubString = otherEventSubTypeStr[evPtr->eventSubtype];
            break;

        case LOG_USER_EVENT:
            eventSubString = userEventSubTypeStr[evPtr->eventSubtype];
            break;

        case LOG_COSEC_EVENT:
            eventSubString = cosecEventSubTypeStr[evPtr->eventSubtype];
            break;

        default:
            return;
	}

    /* Print event information in debug */
    DPRINT(EVENT_LOGGER, "[writeIndex=%d], [isLiveEvt=%s], [isFileWrite=%s], [type=%s], [subType=%s], [state=%d], [detail=%s], [advance=%s]",
           eventCnt->writeIndex, isLiveEvt ? "YES" : "NO", isFileWrite ? "YES" : "NO",
           eventLogTypeStr[evPtr->eventType], eventSubString, evPtr->eventState, evPtr->detail, evPtr->advncDetail);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
