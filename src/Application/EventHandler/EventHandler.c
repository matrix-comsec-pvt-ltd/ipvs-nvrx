//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		EventHandler.c
@brief      Event handling
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "EventHandler.h"
#include "Utils.h"
#include "DebugLog.h"
#include "RecordManager.h"
#include "ImageUpload.h"
#include "SmtpClient.h"
#include "SmsNotify.h"
#include "TcpNotification.h"
#include "FcmPushNotification.h"
#include "CameraInterface.h"
#include "LiveMediaStreamer.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define INVALID_WEEK_SCH	0xFF
#define INVALID_WEEK_DAY	0xFF
#define	DATE_FORMAT			"%02d-%02d-%04d"
#define	DATE_FORMAT_LEN		(10)
#define	TIME_FORMAT			"%02d:%02d:%02d"
#define	TIME_FORMAT_LEN		(8)

#define DO_EVENT_ACTION_STACK_SZ            (4*MEGA_BYTE)
#define RUN_MONITOR_ACTION_STACK_SZ         (4*MEGA_BYTE)

#define TOTAL_CAMERA_EVENT                  (getMaxCameraForCurrentVariant() * MAX_CAMERA_EVENT)
#define GET_CAMERA_EVENT(camIndex, eventNo) ((camIndex * MAX_CAMERA_EVENT) + eventNo);
#define TOTAL_SENSOR_EVENT                  (MAX_SENSOR_EVENT)
#define GET_SENSOR_EVENT(sensorIndex)       (TOTAL_CAMERA_EVENT + sensorIndex)
#define TOTAL_SYSTEM_EVENT                  (MAX_SYSTEM_EVENT)
#define TOTAL_EVENT_ACTION                  (TOTAL_CAMERA_EVENT + TOTAL_SENSOR_EVENT + TOTAL_SYSTEM_EVENT)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    EVNT_START_BEEP,
    EVNT_CAMERA_ALRM_OUTPUT,
    EVNT_SYSTEM_ALRM_OUTPUT,
    EVNT_PTZ_PRESET,
    EVNT_SMS_NOTIFY,
    EVNT_TCP_NOTIFY,
    EVNT_EMAIL_NOTIFY,
    EVNT_IMAGE_UPLOAD,
    EVNT_ALARM_RECORDING,
    EVNT_VIDEO_POPUP, /* Video Pop-Up is dummy action index to match enum values with UI application */
    EVNT_PUSH_NOTIFICATION,
    MAX_EVENT_ACTION
}EVENT_ACTION_e;

typedef struct
{
    BOOL					threadStatus;
    BOOL 					eventStatus;
    UINT16					eventIdx;
    ACTION_BIT_u 			actionBitField;
    ACTION_PARAMETERS_t 	actionParam;
    pthread_mutex_t			threadMutex;
}ACTION_THREAD_t;

typedef struct
{
    BOOL					status;
    UINT8					weekSch;
    UINT8					weekDay;
    BOOL					almRecord[MAX_CAMERA];
    BOOL					imageUpload[MAX_CAMERA];
    BOOL					sysAlrmOutput[MAX_ALARM];
    BOOL					camAlrmOutput[MAX_CAMERA_ALARM];
    pthread_mutex_t			actionMutex;
    ACTION_THREAD_t			actionThread;

}ACTION_STATE_t;

typedef struct
{
    // actionBitfields
    ACTION_BIT_u			actionBitFieldOld;
    ACTION_BIT_u			actionBitFieldNew;

    //Action Param
    ACTION_PARAMETERS_t		newActionParam;
    ACTION_PARAMETERS_t		oldActionParam;

    pthread_mutex_t			runMoniterMutex;

}ACTION_SCHEDULE_OVERLAP_t;

typedef struct
{
    BOOL					status;
    pthread_mutex_t			threadMutex;
}EVENT_STATUS_t;

typedef	enum
{
    EVENT_CAMERA,
    EVENT_SENSOR
}EVENT_MONIOTOR_TYPE_t;

typedef struct
{
    BOOL 					threadStatus;
    UINT8					cameraIdx;
    UINT16					eventIdx;
    EVENT_MONIOTOR_TYPE_t	eventType;
    pthread_mutex_t			moniterMutex;

}EVENT_MONITER_STATE_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void applyConfigChanges(UINT16 eventIdx, ACTION_PARAMETERS_t * oldParaPtr, ACTION_PARAMETERS_t * newParaPtr,
                               ACTION_BIT_u * oldActionPtr, ACTION_BIT_u * newActionPtr);
//-------------------------------------------------------------------------------------------------
static BOOL checkTimeWindow(struct tm * brokenTime,  WEEKLY_ACTION_SCHEDULE_t * weeklySchedule,
                            BOOL * startPreAlrmRec, UINT8PTR weekSchPtr, UINT8PTR weekDayPtr);
//-------------------------------------------------------------------------------------------------
static BOOL systemEventNofity(SYSTEM_EVENT_e sysEvent, UINT8 action);
//-------------------------------------------------------------------------------------------------
static BOOL startAlarmRecord(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx);
//-------------------------------------------------------------------------------------------------
static BOOL sendFcmPushNotification(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx);
//-------------------------------------------------------------------------------------------------
static BOOL startImageUplpad(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx);
//-------------------------------------------------------------------------------------------------
static BOOL emailNotify(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx);
//-------------------------------------------------------------------------------------------------
static BOOL tcpNotify(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx);
//-------------------------------------------------------------------------------------------------
static BOOL smsNotify(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx);
//-------------------------------------------------------------------------------------------------
static BOOL ptzPreset(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx);
//-------------------------------------------------------------------------------------------------
static BOOL systemAlrmOutput(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx);
//-------------------------------------------------------------------------------------------------
static BOOL cameraAlrmOuput(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx);
//-------------------------------------------------------------------------------------------------
static BOOL startBeep(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx);
//-------------------------------------------------------------------------------------------------
static void camAlrmCallback(UINT8 cameraIndex, UINT8 alarmIndex, NET_CMD_STATUS_e status);
//-------------------------------------------------------------------------------------------------
static BOOL checkScheduleEvent(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void checkScheduleStartStopPollEvent(void);
//-------------------------------------------------------------------------------------------------
static void takeEventAction(BOOL eventState, ACTION_BIT_u * actionBitField, ACTION_PARAMETERS_t * actionParam, UINT16 eventIndex, BOOL updateStatus);
//-------------------------------------------------------------------------------------------------
static VOIDPTR doEventAction(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static void onBootTmrCb(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL prepareNotifyMsg(UINT16 eventIdx, CHARPTR inputfmt, CHARPTR outPtr, UINT32 len, const UINT16 outPtrLen);
//-------------------------------------------------------------------------------------------------
static BOOL PrepareFcmPushNotificationMsg(UINT16 eventIdx, PUSH_NOTIFICATION_EVENT_PARAM_t *pEventParam);
//-------------------------------------------------------------------------------------------------
static BOOL getSystemEventStatus(UINT8 sysEvent);
//-------------------------------------------------------------------------------------------------
static VOIDPTR runMonitorAction(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static TIMER_HANDLE					onBootTmrHandler;
static ACTION_STATE_t				actionState[((MAX_CAMERA * MAX_CAMERA_EVENT) + TOTAL_SENSOR_EVENT + TOTAL_SYSTEM_EVENT)];
static EVENT_STATUS_t				systemEvtStatus[MAX_SYSTEM_EVENT];
static ACTION_SCHEDULE_OVERLAP_t	actionScheduleOverlap[((MAX_CAMERA * MAX_CAMERA_EVENT) + TOTAL_SENSOR_EVENT + TOTAL_SYSTEM_EVENT)];
static BOOL                         configUpdate = FALSE;
static UINT32                       configEventUpdate = ((MAX_CAMERA * MAX_CAMERA_EVENT) + TOTAL_SENSOR_EVENT + TOTAL_SYSTEM_EVENT);
static pthread_mutex_t              overlapCondMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t               overlapCondSignal = PTHREAD_COND_INITIALIZER;

static BOOL (*actionTaken[MAX_EVENT_ACTION])(ACTION_PARAMETERS_t * actionData,  BOOL activeDeactive, UINT16 eventIdx) =
{
    startBeep,
    cameraAlrmOuput,
    systemAlrmOutput,
    ptzPreset,
    smsNotify,
    tcpNotify,
    emailNotify,
    startImageUplpad,
    startAlarmRecord,
    NULL,
    sendFcmPushNotification,
};

static const CHARPTR evtStatusStr[] =
{
    "Inactive",
    "Active",
};

static const CHARPTR cameraEventName[MAX_CAMERA_EVENT] =
{
    "Motion Detected",
    "Tampering Detected",
    "Camera Sensor 1 Activated",
    "Camera Sensor 2 Activated",
    "Camera Sensor 3 Activated",
    "Camera Offline Event",
    "Recording Failed",
    "Trip Wire Detected",
    "Object Intrusion Detected",
    "Audio Exception Event",
    "Missing Object Event",
    "Suspicious Object Detected",
    "Loitering Detected",
    "Camera Online Event",
    "Recording Started",
    "Object Counting Event",
    "No Motion Detected"
};

static const CHARPTR systemEventName[MAX_SYSTEM_EVENT] =
{
    "Manual Trigger Activated",
    "System Boot-up",
    "Storage Alert",
    "System on UPS",
    "Disk Volume Full",
    "Disk Fault Found",
    "Schedule Backup Failed",
    "Firmware Upgrade Available",
};

// This array is used to check whether have to give video pop up event
// for specific camera event.
static const LOG_EVENT_SUBTYPE_e evntDetailForVideoPopup[MAX_CAMERA_EVENT]=
{
    LOG_MOTION_DETECTION,       /*MOTION_DETECT*/
    LOG_VIEW_TEMPERING,         /*VIEW_TEMPERING*/
    LOG_CAMERA_SENSOR_1,        /*CAMERA_SENSOR_1*/
    LOG_CAMERA_SENSOR_2,        /*CAMERA_SENSOR_2*/
    LOG_CAMERA_SENSOR_3,        /*CAMERA_SENSOR_3*/
    LOG_NO_CAMERA_EVENT,        /*CONNECTION_FAILURE*/
    LOG_RECORDING_FAIL,         /*RECORDING_FAIL*/
    LOG_LINE_CROSS,             /*LINE_CROSS*/
    LOG_OBJECT_INTRUTION,       /*OBJECT_INTRUSION*/
    LOG_AUDIO_EXCEPTION,        /*AUDIO_EXCEPTION*/
    LOG_MISSING_OBJECT,         /*MISSING_OBJECT*/
    LOG_SUSPICIOUS_OBJECT,      /*SUSPICIOUS_OBJECT*/
    LOG_LOITERING,              /*LOITERING*/
    LOG_CAMERA_STATUS,          /*CAMERA_STATUS*/       // That is not Camera event but showing video pop-up we used as used
    LOG_ALARM_RECORDING,        /*RECORDING_START*/
    LOG_OBJECT_COUNTING,        /*OBJECT_COUNTING*/
    LOG_NO_MOTION_DETECTION    /*NO_MOTION_DETECTION*/
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initialize event handle module to its reset condition. It will also start
 *          1 minute timer for checking where to poll or to stop poll for particular event from camera.
 */
void InitEventHandler(void)
{
    INT32 				cnt;
    ONE_MIN_NOTIFY_t	oneMinFun;

    for(cnt = 0; cnt < TOTAL_EVENT_ACTION; cnt++)
    {
        actionState[cnt].status = INACTIVE;
        memset(&actionState[cnt].almRecord, 0, MAX_CAMERA);
        memset(&actionState[cnt].imageUpload, 0, MAX_CAMERA);
        memset(&actionState[cnt].sysAlrmOutput, 0, MAX_ALARM);
        memset(&actionState[cnt].camAlrmOutput, 0, MAX_CAMERA_ALARM);
        MUTEX_INIT(actionState[cnt].actionMutex, NULL);
        actionState[cnt].weekDay = INVALID_WEEK_DAY;
        actionState[cnt].weekSch = INVALID_WEEK_SCH;

        actionState[cnt].actionThread.eventStatus = INACTIVE;
        actionState[cnt].actionThread.threadStatus = INACTIVE;
        MUTEX_INIT(actionState[cnt].actionThread.threadMutex, NULL);

        MUTEX_INIT(actionScheduleOverlap[cnt].runMoniterMutex, NULL);
        memset(&actionScheduleOverlap[cnt].actionBitFieldOld, 0, (sizeof(ACTION_BIT_u)));
        memset(&actionScheduleOverlap[cnt].actionBitFieldNew, 0, (sizeof(ACTION_BIT_u)));

        memset(&actionScheduleOverlap[cnt].oldActionParam, 0, sizeof(ACTION_PARAMETERS_t));
        memset(&actionScheduleOverlap[cnt].newActionParam, 0, sizeof(ACTION_PARAMETERS_t));
    }

    for(cnt = 0; cnt < MAX_SYSTEM_EVENT; cnt++)
    {
        systemEvtStatus[cnt].status = INACTIVE;
        MUTEX_INIT(systemEvtStatus[cnt].threadMutex, NULL);
    }

    oneMinFun.funcPtr = checkScheduleEvent;
    oneMinFun.userData = 0;

    if(RegisterOnMinFun(&oneMinFun) != SUCCESS)
    {
        EPRINT(EVENT_HANDLER, "fail to register one minute function");
    }

    if (FAIL == Utils_CreateThread(NULL, runMonitorAction, NULL, DETACHED_THREAD, RUN_MONITOR_ACTION_STACK_SZ))
    {
        EPRINT(EVENT_HANDLER, "fail to start event/action monitor thread");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function de-initialize event handle module to its reset condition. It will also stop 1 minute timer.
 */
void DeInitEventHandler(void)
{
    UINT8 cnt;

    for(cnt = 0; cnt < MAX_ALARM; cnt++)
    {
        ChangeAlarmStatus(cnt, INACTIVE);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates camera event configuration.
 * @param   newCfg
 * @param   oldCfg
 * @param   camIndex
 * @param   camEventIndex
 */
void EvntHndlrCamEventCfgUpdate(CAMERA_EVENT_CONFIG_t newCfg,  CAMERA_EVENT_CONFIG_t * oldCfg, UINT8 camIndex, UINT8 camEventIndex)
{
    UINT16			eventIdx;
    struct tm 		brokenTime = { 0 };
    ACTION_BIT_u	actionBitFieldOld;

    if((camIndex < getMaxCameraForCurrentVariant()) && (camEventIndex < MAX_CAMERA_EVENT))
    {
        DPRINT(EVENT_HANDLER, "event config change: [camera=%d], [event=%s]", camIndex, cameraEventName[camEventIndex]);

        eventIdx = GET_CAMERA_EVENT(camIndex, camEventIndex);

        if((oldCfg->action == ENABLE) && (newCfg.action == DISABLE))
        {
            StopCameraEventPoll(camIndex, camEventIndex);

            if(AUTO_ADDED_CAMERA == CameraType(camIndex))
            {
                EventDetectFunc(camIndex, camEventIndex, INACTIVE);
            }

            if(SUCCESS != GetLocalTimeInBrokenTm(&brokenTime))
            {
                EPRINT(EVENT_HANDLER, "failed to get local time in broken: [camera=%d]", camIndex);
            }

            CheckEventTimeWindow(&oldCfg->weeklySchedule[brokenTime.tm_wday], &actionBitFieldOld, &brokenTime);
            MUTEX_LOCK(actionState[eventIdx].actionMutex);

            if(actionState[eventIdx].status == ACTIVE)
            {
                MUTEX_UNLOCK(actionState[eventIdx].actionMutex);
                takeEventAction(INACTIVE, &actionBitFieldOld, &oldCfg->actionParam, eventIdx, TRUE);
            }
            else
            {
                MUTEX_UNLOCK(actionState[eventIdx].actionMutex);
            }
        }
    }

    MUTEX_LOCK(overlapCondMutex);
    configUpdate = TRUE;
    configEventUpdate = GET_CAMERA_EVENT(camIndex, camEventIndex);
    pthread_cond_signal(&overlapCondSignal);
    MUTEX_UNLOCK(overlapCondMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates sensor event configuration.
 * @param   newCfg
 * @param   oldCfg
 * @param   sensorIndex
 */
void EvntHndlrSensorEventCfgUpdate(SENSOR_EVENT_CONFIG_t newCfg, SENSOR_EVENT_CONFIG_t * oldCfg, UINT8 sensorIndex)
{
    UINT16			eventIdx;
    struct tm 		brokenTime = { 0 };
    ACTION_BIT_u	actionBitFieldOld;

    if(sensorIndex < MAX_SENSOR)
    {
        DPRINT(EVENT_HANDLER, "sensor config change: [sensor=%d]", sensorIndex);

        eventIdx = TOTAL_CAMERA_EVENT + sensorIndex;

        if((oldCfg->action == ENABLE) && (newCfg.action == DISABLE))
        {
            if(SUCCESS != GetLocalTimeInBrokenTm(&brokenTime))
            {
                EPRINT(EVENT_HANDLER, "failed to get local time in broken");
            }

            CheckEventTimeWindow(&oldCfg->weeklySchedule[brokenTime.tm_wday], &actionBitFieldOld, &brokenTime);
            MUTEX_LOCK(actionState[eventIdx].actionMutex);

            if(actionState[eventIdx].status == ACTIVE)
            {
                MUTEX_UNLOCK(actionState[eventIdx].actionMutex);
                takeEventAction(INACTIVE, &actionBitFieldOld, &oldCfg->actionParam, eventIdx, TRUE);
            }
            else
            {
                MUTEX_UNLOCK(actionState[eventIdx].actionMutex);
            }
        }
    }

    MUTEX_LOCK(overlapCondMutex);
    configUpdate = TRUE;
    configEventUpdate = GET_SENSOR_EVENT(sensorIndex);
    pthread_cond_signal(&overlapCondSignal);
    MUTEX_UNLOCK(overlapCondMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates system event configuration.
 * @param   newCfg
 * @param   oldCfg
 * @param   systemIndex
 */
void EvntHndlrSystemEventCfgUpdate(SYSTEM_EVENT_CONFIG_t newCfg, SYSTEM_EVENT_CONFIG_t * oldCfg, UINT8 systemIndex)
{
    if (systemIndex >= MAX_SYSTEM_EVENT)
    {
        return;
    }

    UINT16 eventIdx = TOTAL_CAMERA_EVENT + TOTAL_SENSOR_EVENT + systemIndex;
    DPRINT(EVENT_HANDLER, "system event config change: [event=%s], [eventIdx=%d]", systemEventName[systemIndex], eventIdx);

    if((oldCfg->action == DISABLE) && (newCfg.action == ENABLE))
    {
        if(getSystemEventStatus(systemIndex) == ACTIVE)
        {
            takeEventAction(ACTIVE, &newCfg.actionBits, &newCfg.actionParam, eventIdx, TRUE);
        }
    }
    else if((oldCfg->action == ENABLE) && (newCfg.action == DISABLE))
    {
        MUTEX_LOCK(actionState[eventIdx].actionMutex);

        if(actionState[eventIdx].status == ACTIVE)
        {
            MUTEX_UNLOCK(actionState[eventIdx].actionMutex);
            takeEventAction(INACTIVE, &oldCfg->actionBits, &oldCfg->actionParam, eventIdx, TRUE);
        }
        else
        {
            MUTEX_UNLOCK(actionState[eventIdx].actionMutex);
        }
    }
    else
    {
        applyConfigChanges(eventIdx, &oldCfg->actionParam, &newCfg.actionParam, &oldCfg->actionBits, &newCfg.actionBits);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   applyConfigChanges
 * @param   eventIdx
 * @param   oldParaPtr
 * @param   newParaPtr
 * @param   oldActionPtr
 * @param   newActionPtr
 */
static void applyConfigChanges(UINT16 eventIdx, ACTION_PARAMETERS_t * oldParaPtr, ACTION_PARAMETERS_t * newParaPtr,
                               ACTION_BIT_u * oldActionPtr, ACTION_BIT_u * newActionPtr)
{
    BOOL					eventDisable;
    EVENT_ACTION_e			evtAct;
    UINT8					cnt;
    UINT16					oldActData, newActData;
    ACTION_BIT_u			actionBits;
    ACTION_BIT_u			startActionBits;
    ACTION_PARAMETERS_t		stopActionPara;
    ACTION_PARAMETERS_t		startActionPara;

    MUTEX_LOCK(actionState[eventIdx].actionMutex);
    if (actionState[eventIdx].status != ACTIVE)
    {
        MUTEX_UNLOCK(actionState[eventIdx].actionMutex);
        return;
    }

    actionBits.actionBitGroup = 0;
    startActionBits.actionBitGroup = 0;
    memset(&stopActionPara, 0, (sizeof(ACTION_PARAMETERS_t)));
    memset(&startActionPara, 0, (sizeof(ACTION_PARAMETERS_t)));

    for(evtAct = EVNT_START_BEEP; evtAct < MAX_EVENT_ACTION; evtAct++)
    {
        if (evtAct == EVNT_VIDEO_POPUP)
        {
            continue;
        }

        actionBits.actionBitGroup |= (1 << evtAct);

        oldActData = GET_BIT(oldActionPtr->actionBitGroup, evtAct);
        newActData = GET_BIT(newActionPtr->actionBitGroup, evtAct);

        if((oldActData == DISABLE) && (newActData == ENABLE))
        {
            // Do nothing
            continue;
        }
        else if((oldActData == ENABLE) && (newActData == DISABLE))
        {
            eventDisable = TRUE;
        }
        else
        {
            eventDisable = FALSE;
        }

        if(newActData == ENABLE)
        {
            startActionBits.actionBitGroup |= (1 << evtAct);
        }

        if(evtAct == EVNT_ALARM_RECORDING)
        {
            for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if((oldActData == ENABLE) && (newActData == ENABLE))
                {
                    if((oldParaPtr->alarmRecord[cnt] == ENABLE) && (newParaPtr->alarmRecord[cnt] == ENABLE) && (GetRecordStatus(cnt,ALARM_RECORD) == START))
                    {
                        // Do not take any action, action already taken
                        EPRINT(EVENT_HANDLER, "nothing to do as alarm recording action already taken: [camera=%d]", cnt);
                        continue;
                    }
                }

                if(actionState[eventIdx].almRecord[cnt] == ON)
                {
                    if((newParaPtr->alarmRecord[cnt] == DISABLE) || (eventDisable == TRUE))
                    {
                        stopActionPara.alarmRecord[cnt] = ON;
                    }
                }

                if(actionState[eventIdx].status == ACTIVE)
                {
                    if((newParaPtr->alarmRecord[cnt] == ENABLE) && (eventDisable == FALSE))
                    {
                        startActionPara.alarmRecord[cnt] = ON;
                    }
                }
            }
        }
        else if(evtAct == EVNT_IMAGE_UPLOAD)
        {
            for(cnt = 0; cnt < getMaxCameraForCurrentVariant(); cnt++)
            {
                if((oldActData == ENABLE) && (newActData == ENABLE))
                {
                    if((oldParaPtr->uploadImage[cnt] == ENABLE) && (newParaPtr->uploadImage[cnt] == ENABLE))
                    {
                        // Do not take any action, action already taken
                        EPRINT(EVENT_HANDLER, "nothing to do as image upload action already taken: [camera=%d]", cnt);
                        continue;
                    }
                }

                if(actionState[eventIdx].imageUpload[cnt] == ON)
                {
                    if((newParaPtr->uploadImage[cnt] == DISABLE) || (eventDisable == TRUE))
                    {
                        stopActionPara.uploadImage[cnt] = ON;
                    }
                }

                if(actionState[eventIdx].status == ACTIVE)
                {
                    if((newParaPtr->uploadImage[cnt] == ENABLE) && (eventDisable == FALSE))
                    {
                        startActionPara.uploadImage[cnt] = ON;
                    }
                }
            }
        }
        else if(evtAct == EVNT_SYSTEM_ALRM_OUTPUT)
        {
            for(cnt = 0; cnt < MAX_ALARM; cnt++)
            {
                if((oldActData == ENABLE) && (newActData == ENABLE))
                {
                    if((oldParaPtr->systemAlarmOutput[cnt] == ENABLE) && (newParaPtr->systemAlarmOutput[cnt] == ENABLE))
                    {
                        // Do not take any action, action already taken
                        EPRINT(EVENT_HANDLER, "nothing to do as system alarm output action already taken: [camera=%d]", cnt);
                        continue;
                    }
                }

                if(actionState[eventIdx].sysAlrmOutput[cnt] == ON)
                {
                    if((newParaPtr->systemAlarmOutput[cnt] == DISABLE) || (eventDisable == TRUE))
                    {
                        stopActionPara.systemAlarmOutput[cnt] = ON;
                    }
                }

                if(actionState[eventIdx].status == ACTIVE)
                {
                    if((newParaPtr->systemAlarmOutput[cnt] == ENABLE) && (eventDisable == FALSE))
                    {
                        startActionPara.systemAlarmOutput[cnt] = ON;
                    }
                }
            }
        }
        else if(evtAct == EVNT_CAMERA_ALRM_OUTPUT)
        {
            if((oldParaPtr->cameraAlarmOutput.cameraNumber != newParaPtr->cameraAlarmOutput.cameraNumber) || (eventDisable == TRUE))
            {
                for(cnt = 0; cnt < MAX_CAMERA_ALARM; cnt++)
                {
                    stopActionPara.cameraAlarmOutput.cameraNumber = oldParaPtr->cameraAlarmOutput.cameraNumber;
                    stopActionPara.cameraAlarmOutput.alarm[cnt] = ON;

                    if(actionState[eventIdx].status == ACTIVE)
                    {
                        if((newParaPtr->cameraAlarmOutput.alarm[cnt] == ENABLE) && (eventDisable == FALSE))
                        {
                            startActionPara.cameraAlarmOutput.cameraNumber = newParaPtr->cameraAlarmOutput.cameraNumber;
                            startActionPara.cameraAlarmOutput.alarm[cnt] = ON;
                        }
                    }
                }
            }
            else
            {
                for(cnt = 0; cnt < MAX_CAMERA_ALARM; cnt++)
                {
                    if(actionState[eventIdx].camAlrmOutput[cnt] == ON)
                    {
                        if((newParaPtr->cameraAlarmOutput.alarm[cnt] == DISABLE) || (eventDisable == TRUE))
                        {
                            stopActionPara.cameraAlarmOutput.cameraNumber = newParaPtr->cameraAlarmOutput.cameraNumber;
                            stopActionPara.cameraAlarmOutput.alarm[cnt] = ON;
                        }
                    }
                }
            }
        }
    }
    MUTEX_UNLOCK(actionState[eventIdx].actionMutex);

    takeEventAction(INACTIVE, &actionBits, &stopActionPara, eventIdx, FALSE);
    sleep(2);
    takeEventAction(ACTIVE, &startActionBits, &startActionPara, eventIdx, FALSE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API use to get the update of whenever sensor was activate or de-activate.
 * @param   camIndex
 * @param   eventNo
 * @param   camEvntState
 */
void CameraEventNotify(UINT8 camIndex, UINT8 eventNo, BOOL camEvntState)
{
    CAMERA_EVENT_CONFIG_t cameraEventCfg;
    ACTION_BIT_u actionBitField;
    struct tm brokenTime;
    UINT16 eventIdxNo = GET_CAMERA_EVENT(camIndex, eventNo);
    CHAR detail[MAX_EVENT_DETAIL_SIZE];
    CHAR advDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];
    CAMERA_CONFIG_t cameraCfg;
    BOOL sendVideoPopupEvent = FALSE;

    if ((camIndex >= getMaxCameraForCurrentVariant()) || (eventNo >= MAX_CAMERA_EVENT))
    {
        return;
    }

    // this switch case use for STOP only previous online or offline event.
    if ((eventNo == CAMERA_ONLINE) && (ACTIVE == camEvntState))
    {
        ReadSingleCameraEventConfig(camIndex, CONNECTION_FAILURE, &cameraEventCfg);
        if (cameraEventCfg.action == ENABLE)
        {
            UINT16 prevEvtIdxNo = GET_CAMERA_EVENT(camIndex, CONNECTION_FAILURE);
            if((SUCCESS == GetLocalTimeInBrokenTm(&brokenTime)) &&
                (CheckEventTimeWindow(&cameraEventCfg.weeklySchedule[brokenTime.tm_wday], &actionBitField, &brokenTime) == SUCCESS))
            {
                DPRINT(EVENT_HANDLER, "stop event: [camera=%d], [event=%s], [evtStatus=%s], [prevEvtIdxNo=%d]",
                       camIndex, cameraEventName[CONNECTION_FAILURE], evtStatusStr[camEvntState], prevEvtIdxNo);
                takeEventAction(!camEvntState, &actionBitField, &cameraEventCfg.actionParam, prevEvtIdxNo, TRUE);
            }
        }
    }

    memset(&cameraEventCfg, 0, (sizeof(CAMERA_EVENT_CONFIG_t)));
    ReadSingleCameraEventConfig(camIndex, eventNo, &cameraEventCfg);

    // take action for newly evnt.
    if (cameraEventCfg.action != ENABLE)
    {
        return;
    }

    if ((eventNo == MOTION_DETECT) || (eventNo == NO_MOTION_DETECTION))
    {
        CamMotionNotify(camIndex, camEvntState);
        ReadSingleCameraConfig(camIndex, &cameraCfg);
    }

    if((SUCCESS == GetLocalTimeInBrokenTm(&brokenTime)) &&
        (CheckEventTimeWindow(&cameraEventCfg.weeklySchedule[brokenTime.tm_wday], &actionBitField, &brokenTime) == SUCCESS))
    {
        DPRINT(EVENT_HANDLER, "[camera=%d], [event=%s], [evtStatus=%s], [eventIdx=%d]",
               camIndex, cameraEventName[eventNo], evtStatusStr[camEvntState], eventIdxNo);
        takeEventAction(camEvntState, &actionBitField, &cameraEventCfg.actionParam, eventIdxNo, TRUE);

        if ((camEvntState == ACTIVE) && ((UINT8)actionBitField.actionBitField.startBeep == TRUE))
        {
            snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camIndex));
            WriteEvent(LOG_OTHER_EVENT, LOG_BUZZER_STATUS, detail, cameraEventName[eventNo], EVENT_RINGING);
        }

        /* Send Video-popup signal(event) if video-popup is configured.
         * If video-popup is enabled in evntDetailForVideoPopup[] list then we will send a specific camera event
         * to Client(Local and Device) regarding video-popup. When video-popup event is received at client end,
         * it will perform video-popup action. */
        if ((eventNo == MOTION_DETECT) || (eventNo == NO_MOTION_DETECTION))
        {
            if ((cameraCfg.camera == TRUE) && (cameraCfg.motionDetectionStatus == TRUE))
            {
                sendVideoPopupEvent = TRUE;
            }
        }
        else
        {
            sendVideoPopupEvent = TRUE;
        }

        if (TRUE == sendVideoPopupEvent)
        {
            if ((camEvntState == ACTIVE) && ((UINT8) actionBitField.actionBitField.videoPopUp == TRUE)
                && (evntDetailForVideoPopup[eventNo] != LOG_NO_CAMERA_EVENT))
            {
                snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camIndex));
                snprintf(advDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%02d", ((UINT8) evntDetailForVideoPopup[eventNo]));
                WriteEvent(LOG_CAMERA_EVENT, LOG_VIDEO_POP_UP, detail, advDetail, EVENT_ACTIVE);
            }
        }
    }
    else
    {
        DPRINT(EVENT_HANDLER, "time window not match: [camera=%d], [event=%s], [eventIdx=%d]", camIndex, cameraEventName[eventNo], eventIdxNo);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API use to get the update of whenever sensor was activate or de-activate.
 * @param   sensorIndex
 * @param   sensorActive
 */
void SensorEventNotify(UINT8 sensorIndex, BOOL sensorActive)
{
    SENSOR_EVENT_CONFIG_t	sensorAction;
    ACTION_BIT_u			actionBitField;
    struct tm 				brokenTime = { 0 };
    UINT16					eventIdxNo = GET_SENSOR_EVENT(sensorIndex);

    if (sensorIndex >= MAX_SENSOR)
    {
        return;
    }

    DPRINT(EVENT_HANDLER, "input sensor: [sensor=%d], [status=%s], [eventIdx=%d]", sensorIndex, evtStatusStr[sensorActive], eventIdxNo);
    ReadSingleSensorEventConfig(sensorIndex, &sensorAction);

    if (sensorAction.action != ON)
    {
        return;
    }

    if((SUCCESS == GetLocalTimeInBrokenTm(&brokenTime)) &&
            (CheckEventTimeWindow(&sensorAction.weeklySchedule[brokenTime.tm_wday], &actionBitField, &brokenTime) == SUCCESS))
    {
        takeEventAction(sensorActive, &actionBitField, &sensorAction.actionParam, eventIdxNo, TRUE);

        if((actionBitField.actionBitField.startBeep == TRUE) && (sensorActive == ACTIVE))
        {
            WriteEvent(LOG_OTHER_EVENT, LOG_BUZZER_STATUS,NULL, "Device Sensor Trigger", EVENT_RINGING);
        }
    }
    else
    {
        DPRINT(EVENT_HANDLER, "time window does not match: [sensor=%d]", sensorIndex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   CheckEventTimeWindow
 * @param   weekAction
 * @param   actionField
 * @param   brokenTimePtr
 * @return
 */
BOOL CheckEventTimeWindow(WEEKLY_ACTION_SCHEDULE_t * weekAction, ACTION_BIT_u * actionField, struct tm * brokenTimePtr)
{
    BOOL						startAction = FAIL;
    UINT8						eventSchdlCnt;
    TIME_HH_MM_t				compareTime;
    ACTION_CONTROL_PARAMETER_t	*actionParam;
    ACTION_BIT_u				tempFileds;
    EVENT_ACTION_e				evtAct;

    if(weekAction->actionEntireDay == ON)
    {
        memcpy(actionField, &weekAction->entireDayAction, sizeof(ACTION_BIT_u));
        return SUCCESS;
    }

    compareTime.hour = brokenTimePtr->tm_hour;
    compareTime.minute = brokenTimePtr->tm_min;
    tempFileds.actionBitGroup = 0;

    for(eventSchdlCnt = 0; eventSchdlCnt < MAX_EVENT_SCHEDULE; eventSchdlCnt++)
    {
        actionParam = &weekAction->actionControl[eventSchdlCnt];
        if (actionParam->scheduleAction.actionBitGroup == 0)
        {
            continue;
        }

        if (IsGivenTimeInWindow(compareTime, actionParam->startTime, actionParam->endTime) != YES)
        {
            continue;
        }

        if((UINT8)actionParam->scheduleAction.actionBitField.startBeep == TRUE)
        {
            tempFileds.actionBitField.startBeep = TRUE;
        }

        if((UINT8)actionParam->scheduleAction.actionBitField.cameraAlarmOutput == TRUE)
        {
            tempFileds.actionBitField.cameraAlarmOutput = TRUE;
        }

        if((UINT8)actionParam->scheduleAction.actionBitField.systemAlarmOutput == TRUE)
        {
            tempFileds.actionBitField.systemAlarmOutput = TRUE;
        }

        if((UINT8)actionParam->scheduleAction.actionBitField.gotoPresetPtz == TRUE)
        {
            tempFileds.actionBitField.gotoPresetPtz = TRUE;
        }

        if((UINT8)actionParam->scheduleAction.actionBitField.smsNotification == TRUE)
        {
            tempFileds.actionBitField.smsNotification = TRUE;
        }

        if((UINT8)actionParam->scheduleAction.actionBitField.tcpNotification == TRUE)
        {
            tempFileds.actionBitField.tcpNotification = TRUE;
        }

        if((UINT8)actionParam->scheduleAction.actionBitField.emailNotification == TRUE)
        {
            tempFileds.actionBitField.emailNotification = TRUE;
        }

        if((UINT8)actionParam->scheduleAction.actionBitField.alarmRecording == TRUE)
        {
            tempFileds.actionBitField.alarmRecording = TRUE;
        }

        if((UINT8)actionParam->scheduleAction.actionBitField.videoPopUp == TRUE)
        {
            tempFileds.actionBitField.videoPopUp = TRUE;
        }

        if((UINT8)actionParam->scheduleAction.actionBitField.pushNotification == TRUE)
        {
            tempFileds.actionBitField.pushNotification = TRUE;
        }

        for(evtAct = EVNT_START_BEEP; evtAct < MAX_EVENT_ACTION; evtAct++)
        {
            if (evtAct == EVNT_VIDEO_POPUP)
            {
                continue;
            }

            if (GET_BIT(actionParam->scheduleAction.actionBitGroup, evtAct) == TRUE)
            {
                tempFileds.actionBitGroup |= ((0x01) << evtAct);
            }
        }

        startAction = SUCCESS;
    }

    if (startAction == SUCCESS)
    {
        memcpy(actionField, &tempFileds, sizeof(ACTION_BIT_u));
    }

    return startAction;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is called to check if action is scheduled for given time frame or not.
 *          It will also tell if alarm recording was enable or not.
 * @param   brokenTime
 * @param   weeklySchedule
 * @param   startPreAlrmRec
 * @param   weekSchPtr
 * @param   weekDayPtr
 * @return
 */
static BOOL checkTimeWindow(struct tm * brokenTime,  WEEKLY_ACTION_SCHEDULE_t * weeklySchedule, BOOL * startPreAlrmRec, UINT8PTR weekSchPtr, UINT8PTR weekDayPtr)
{
    BOOL			actionCntrlCnt;
    TIME_HH_MM_t	timeWindow;

    *weekSchPtr = INVALID_WEEK_SCH;
    *weekDayPtr = INVALID_WEEK_DAY;

    // check entire day action was enable
    if(weeklySchedule->actionEntireDay == ENABLE)
    {
        if(weeklySchedule->entireDayAction.actionBitField.alarmRecording == TRUE)
        {
            *startPreAlrmRec = YES;
        }
        else
        {
            *startPreAlrmRec = NO;
        }

        *weekSchPtr = MAX_EVENT_SCHEDULE;
        *weekDayPtr = brokenTime->tm_wday;
        return YES;
    }

    timeWindow.hour = brokenTime->tm_hour;
    timeWindow.minute = brokenTime->tm_min;

    *startPreAlrmRec = NO;

    // entire day was not enable so checking each 6 schedule event time
    for(actionCntrlCnt = 0; actionCntrlCnt < MAX_EVENT_SCHEDULE; actionCntrlCnt++)
    {
        if(IsGivenTimeInWindow(timeWindow, weeklySchedule->actionControl[actionCntrlCnt].startTime,
                               weeklySchedule->actionControl[actionCntrlCnt].endTime) == YES)
        {
            if(weeklySchedule->actionControl[actionCntrlCnt] .scheduleAction.actionBitField.alarmRecording == TRUE)
            {
                *startPreAlrmRec = YES;
            }

            *weekSchPtr = actionCntrlCnt;
            *weekDayPtr = brokenTime->tm_wday;
            return YES;
        }
    }

    return NO;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets event of hard disk full from disk manager and takes appropriate
 *          action from the configuration.
 * @param   action
 */
void HarDiskFull(UINT8 action)
{
    systemEventNofity(SYS_EVT_DISK_VOLUME_FULL, action);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets event of storage alert of certain limit from disk manager and takes
 *          appropriate action from the configuration.
 * @param   action
 */
void StorageAlert(UINT8	action)
{
    systemEventNofity(SYS_EVT_STORAGE_ALERT, action);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets event of disk fault from disk manager and takes appropriate action
 *          from the configuration.
 * @param   action
 */
void DiskFaultEvent(UINT8 action)
{
    systemEventNofity(SYS_EVT_DISK_FAULT, action);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets status of manual trigger.
 * @param   temp1
 * @param   temp2
 * @return
 */
BOOL GetManTriggerStatus(UINT8 temp1, UINT8 temp2)
{
    return(getSystemEventStatus(SYS_EVT_MANUAL_TRIGGER));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets event of manual trigger start or stop and takes action as per
 *          configured by user.
 * @param   action
 * @return
 */
NET_CMD_STATUS_e ManualTrigger(UINT8 action)
{
    if (systemEventNofity(SYS_EVT_MANUAL_TRIGGER, action) == SUCCESS)
    {
        return CMD_SUCCESS;
    }

    if(GetManTriggerStatus(0, 0) == ON)
    {
        return CMD_MAN_TRG_ALREADY_ON;
    }
    else
    {
        return CMD_MAN_TRG_ALREADY_OFF;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets event of on boot generated by system itself and takes action as
 *          per configured by user.
 * @param   action
 */
void OnBootEvent(UINT8	action)
{
    TIMER_INFO_t timerInfo;

    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(30);
    timerInfo.data = action;
    timerInfo.funcPtr = onBootTmrCb;
    StartTimer(timerInfo, &onBootTmrHandler);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Firmware upgrade available notification
 * @param   action
 */
void FirmwareUpgradeEvent(UINT8 action)
{
    systemEventNofity(SYS_EVT_FIRMWARE_UPGRADE, action);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is callback of timer to deactivate On boot event
 * @param   data
 */
static void onBootTmrCb(UINT32	data)
{
    static BOOL tempF = INACTIVE;

    if (tempF == INACTIVE)
    {
        tempF = ACTIVE;
        ReloadTimer(onBootTmrHandler, CONVERT_SEC_TO_TIMER_COUNT(5));
    }
    else
    {
        tempF = INACTIVE;
        DeleteTimer(&onBootTmrHandler);
    }
    systemEventNofity(SYS_EVT_ON_BOOT, tempF);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets event whenever system goes on UPS and takes action against it.
 * @param   sysEvent
 * @param   action
 * @return
 */
static BOOL systemEventNofity(SYSTEM_EVENT_e sysEvent, UINT8 action)
{
    SYSTEM_EVENT_CONFIG_t	systemEventConfig;
    UINT16					eventIdxNo = (TOTAL_CAMERA_EVENT + TOTAL_SENSOR_EVENT + sysEvent);

    DPRINT(EVENT_HANDLER, "system event: [event=%s], [status=%s], [eventIdx=%d]", systemEventName[sysEvent], evtStatusStr[action], eventIdxNo);
    MUTEX_LOCK(systemEvtStatus[sysEvent].threadMutex);

    if(systemEvtStatus[sysEvent].status == action)
    {
        MUTEX_UNLOCK(systemEvtStatus[sysEvent].threadMutex);
        return FAIL;
    }

    systemEvtStatus[sysEvent].status = action;
    MUTEX_UNLOCK(systemEvtStatus[sysEvent].threadMutex);

    ReadSingleSystemEventConfig(sysEvent, &systemEventConfig);

    if(systemEventConfig.action == ENABLE)
    {
        takeEventAction(action, &systemEventConfig.actionBits, &systemEventConfig.actionParam, eventIdxNo, TRUE);
        if((action == TRUE)  && ((UINT8)systemEventConfig.actionBits.actionBitField.startBeep == TRUE))
        {
            WriteEvent(LOG_OTHER_EVENT, LOG_BUZZER_STATUS, NULL, systemEventName[sysEvent], EVENT_RINGING);
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function starts or stop alarm recording.
 * @param   actionData
 * @param   activeDeactive
 * @param   eventIdx
 * @return
 */
static BOOL startAlarmRecord(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx)
{
    UINT8 cameraCnt;

    // check for each camera whether to start alarm recording or not
    for(cameraCnt = 0; cameraCnt < getMaxCameraForCurrentVariant(); cameraCnt++)
    {
        if(actionData->alarmRecord[cameraCnt] != ON)
        {
            continue;
        }

        // start alarm recording
        if(activeDeactive == ACTIVE)
        {
            DPRINT(EVENT_HANDLER, "start alarm record: [camera=%d]", cameraCnt);
            StartRecord(cameraCnt, ALARM_RECORD, DEFAULT_REC_DURATION, NULL);

        }
        else
        {
            DPRINT(EVENT_HANDLER, "stop alarm record: [camera=%d]", cameraCnt);
            StopRecord(cameraCnt, ALARM_RECORD, NULL);
        }

        MUTEX_LOCK(actionState[eventIdx].actionMutex);
        actionState[eventIdx].almRecord[cameraCnt] = activeDeactive;
        MUTEX_UNLOCK(actionState[eventIdx].actionMutex);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function upload image.
 * @param   actionData
 * @param   activeDeactive
 * @param   eventIdx
 * @return
 */
static BOOL startImageUplpad(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx)
{
    UINT8 cameraCnt;

    // check for each camera whether to start image uploading or not
    for(cameraCnt = 0; cameraCnt < getMaxCameraForCurrentVariant(); cameraCnt++)
    {
        if(actionData->uploadImage[cameraCnt] != ON)
        {
            continue;
        }

        if(activeDeactive == ACTIVE)
        {
            DPRINT(EVENT_HANDLER, "start image upload: [camera=%d]", cameraCnt);
            StartImageUpload(cameraCnt);
        }
        else
        {
            DPRINT(EVENT_HANDLER, "stop image upload: [camera=%d]", cameraCnt);
            StopImageUpload(cameraCnt);
        }

        MUTEX_LOCK(actionState[eventIdx].actionMutex);
        actionState[eventIdx].imageUpload[cameraCnt] = activeDeactive;
        MUTEX_UNLOCK(actionState[eventIdx].actionMutex);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sends push notification to Android/iOS device using FCM service
 * @param   actionData
 * @param   activeDeactive
 * @param   eventIdx
 * @return
 */
static BOOL sendFcmPushNotification(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx)
{
    PUSH_NOTIFICATION_EVENT_PARAM_t eventParam;

    /* for normal state push notification will not be sent */
    if (activeDeactive != ACTIVE)
    {
        return FAIL;
    }

    /* prepare notification payload */
    if (SUCCESS != PrepareFcmPushNotificationMsg(eventIdx, &eventParam))
    {
        return FAIL;
    }

    /* pass event parameters to push notification module */
    if (CMD_SUCCESS != ProcessFcmPushNotification(&eventParam))
    {
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function prepares message for all notification function. Where user can input some
 *          short format like %E = Event name etc.
 * @param   eventIdx
 * @param   inputfmt
 * @param   outPtr
 * @param   len
 * @param   outPtrLen
 * @return
 */
static BOOL prepareNotifyMsg(UINT16 eventIdx, CHARPTR inputfmt, CHARPTR outPtr, UINT32 len, const UINT16 outPtrLen)
{
    CHAR				value;
    CHARPTR				tmpPtr ,resPtr;
    UINT8				majorIdx, minorIdx;
    UINT32				outLen = 0, sprintLen = 0;
    INT32               tmpLen = 0;
    CHAR				formatStr[150] = { '\0' };
    struct tm 			localTime = { 0 };
    GENERAL_CONFIG_t	genCfg;
    CAMERA_CONFIG_t		camCfg;
    SENSOR_CONFIG_t		sensorCfg;

    if ((inputfmt == NULL) || (outPtr == NULL))
    {
        return FAIL;
    }

    if (SUCCESS != GetLocalTimeInBrokenTm(&localTime))
    {
        EPRINT(EVENT_HANDLER, "failed to get local time in broken");
    }

    tmpPtr = inputfmt;

    while (((resPtr = strchr(tmpPtr, '%')) != NULL) && (strlen(outPtr) < len))
    {
        value = resPtr[1];
        tmpLen = (strlen(tmpPtr) - strlen(resPtr));
        if ((tmpLen < 0) || (tmpLen > (INT32)(len - outLen - 1)))
        {
            EPRINT(EVENT_HANDLER, "insufficient buffer");
            return FAIL;
        }

        if (tmpLen > 0)
        {
            snprintf(outPtr + outLen, tmpLen+1, "%s", tmpPtr);
            outLen += tmpLen;
        }

        if(strlen(resPtr) >= 2)
        {
            tmpPtr = resPtr + 2;
        }
        else
        {
            tmpPtr = resPtr + 1;
        }

        switch(value)
        {
            case 'E':
            {
                if(eventIdx < TOTAL_CAMERA_EVENT)
                {
                    majorIdx = ((eventIdx / MAX_CAMERA_EVENT) + 1);
                    minorIdx = (eventIdx % MAX_CAMERA_EVENT);
                    ReadSingleCameraConfig(majorIdx - 1, &camCfg);
                    snprintf(formatStr, sizeof(formatStr), "%s (Camera-%d: %s)", cameraEventName[minorIdx], majorIdx, camCfg.name);
                }
                else if((eventIdx >= TOTAL_CAMERA_EVENT) && (eventIdx < (TOTAL_SENSOR_EVENT + TOTAL_CAMERA_EVENT)))
                {
                    majorIdx = (eventIdx - TOTAL_CAMERA_EVENT);
                    ReadSingleSensorConfig(majorIdx, &sensorCfg);
                    snprintf(formatStr, sizeof(formatStr), "Sensor Input: %s Activated", sensorCfg.name);
                }
                else if(eventIdx >= (TOTAL_SENSOR_EVENT + TOTAL_CAMERA_EVENT))
                {
                    majorIdx = (eventIdx - TOTAL_CAMERA_EVENT - TOTAL_SENSOR_EVENT);
                    snprintf(formatStr, sizeof(formatStr), "%s", systemEventName[majorIdx]);
                }

                if((outLen + strlen(formatStr)) < len)
                {
                    sprintLen = snprintf(outPtr + outLen, outPtrLen - outLen, "%s", formatStr);
                    outLen += sprintLen;
                }
            }
            break;

            case 'D':
            {
                if((outLen + DATE_FORMAT_LEN) < len)
                {
                    sprintLen = snprintf(outPtr + outLen, outPtrLen - outLen, DATE_FORMAT, localTime.tm_mday, (localTime.tm_mon + 1), localTime.tm_year);
                    outLen += sprintLen;
                }
            }
            break;

            case 'd':
            {
                ReadGeneralConfig(&genCfg);
                if((outLen + strlen(genCfg.deviceName)) < len)
                {
                    sprintLen = snprintf(outPtr + outLen, outPtrLen - outLen, "%s", genCfg.deviceName);
                    outLen += sprintLen;
                }
            }
            break;

            case 'T':
            {
                if((outLen + TIME_FORMAT_LEN) < len)
                {
                    sprintLen = snprintf(outPtr + outLen, outPtrLen - outLen, TIME_FORMAT, localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
                    outLen += sprintLen;
                }
            }
            break;

            case '%':
            {
                if((outLen + 2) < len)
                {
                    sprintLen = snprintf(outPtr + outLen, outPtrLen - outLen, "%%%%");
                    outLen += sprintLen;
                }
            }
            break;

            default:
            {
                if((outLen + 2) < len)
                {
                    sprintLen = snprintf(outPtr + outLen, outPtrLen - outLen, "%%%c", value);
                    outLen += sprintLen;
                }
            }
            break;
        }
    }

    if(outLen >= (len-1))
    {
        outPtr[len-1]='\0';
    }
    else
    {
        outPtr[outLen]='\0';
    }

    if(outLen > outPtrLen)
    {
        EPRINT(EVENT_HANDLER, "length is greater than buffer: [outLen=%d]", outLen);
        outLen = outPtrLen;
    }

    if((outLen + strlen(tmpPtr)) < len)
    {
        sprintLen = snprintf(outPtr + outLen, outPtrLen - outLen, "%s", tmpPtr);
        outLen += sprintLen;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief PrepareFcmPushNotificationMsg
 * @param eventIdx
 * @param pEventParam
 * @return
 */
static BOOL PrepareFcmPushNotificationMsg(UINT16 eventIdx, PUSH_NOTIFICATION_EVENT_PARAM_t *pEventParam)
{
    UINT8 majorIdx, minorIdx;
    CAMERA_CONFIG_t camCfg;
    SENSOR_CONFIG_t sensorCfg;
    GENERAL_CONFIG_t generalConfig;
    struct tm localTime = { 0 };

    /* set date/time of an event */
    GetLocalTimeInBrokenTm(&localTime);

    /* read general configuration for device name */
    ReadGeneralConfig(&generalConfig);

    /* copy event date/time */
    snprintf(pEventParam->dateTime, sizeof(pEventParam->dateTime), DATE_FORMAT " " TIME_FORMAT,
             localTime.tm_mday, (localTime.tm_mon + 1), localTime.tm_year,
             localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

    /* camera events */
    if (eventIdx < TOTAL_CAMERA_EVENT)
    {
        majorIdx = (eventIdx / MAX_CAMERA_EVENT);
        minorIdx = (eventIdx % MAX_CAMERA_EVENT);
        ReadSingleCameraConfig(majorIdx, &camCfg);

        snprintf(pEventParam->title, sizeof(pEventParam->title),
                 "%s" " (" DATE_FORMAT " " TIME_FORMAT ")",
                 cameraEventName[minorIdx],
                 localTime.tm_mday, (localTime.tm_mon + 1), localTime.tm_year,
                 localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

        snprintf(pEventParam->body, sizeof(pEventParam->body), "%s | %s", generalConfig.deviceName, camCfg.name);
    }
    /* sensor events */
    else if((eventIdx >= TOTAL_CAMERA_EVENT) && (eventIdx < (TOTAL_SENSOR_EVENT + TOTAL_CAMERA_EVENT)))
    {
        majorIdx = (eventIdx - TOTAL_CAMERA_EVENT);
        ReadSingleSensorConfig(majorIdx, &sensorCfg);

        snprintf(pEventParam->title, sizeof(pEventParam->title),
                 "%s" " (" DATE_FORMAT " " TIME_FORMAT ")",
                 "Sensor Input Activated",
                 localTime.tm_mday, (localTime.tm_mon + 1), localTime.tm_year,
                 localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

        snprintf(pEventParam->body, sizeof(pEventParam->body), "%s | %s", generalConfig.deviceName, sensorCfg.name);
    }
    /* system events */
    else if(eventIdx >= (TOTAL_SENSOR_EVENT + TOTAL_CAMERA_EVENT))
    {
        majorIdx = (eventIdx - TOTAL_CAMERA_EVENT - TOTAL_SENSOR_EVENT);
        snprintf(pEventParam->title, sizeof(pEventParam->title),
                 "%s" " (" DATE_FORMAT " " TIME_FORMAT ")",
                 systemEventName[majorIdx],
                 localTime.tm_mday, (localTime.tm_mon + 1), localTime.tm_year,
                 localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

        snprintf(pEventParam->body, sizeof(pEventParam->body), "%s", generalConfig.deviceName);
    }
    else
    {
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sens email notification.
 * @param   actionData
 * @param   activeDeactive
 * @param   eventIdx
 * @return
 */
static BOOL emailNotify(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx)
{
    EMAIL_PARAMETER_t emailParam;

    memset(&emailParam,0,sizeof(EMAIL_PARAMETER_t));
    if (activeDeactive != ACTIVE)
    {
        return FAIL;
    }

    snprintf(emailParam.emailAddress, MAX_EMAIL_ADDRESS_WIDTH, "%s", actionData->sendEmail.emailAddress );

    if (FAIL == prepareNotifyMsg(eventIdx, actionData->sendEmail.subject, emailParam.subject, MAX_EMAIL_SUBJECT_WIDTH, sizeof(emailParam.subject)))
    {
        EPRINT(EVENT_HANDLER, "fail to prepare email subject");
        return FAIL;
    }

    if (FAIL == prepareNotifyMsg(eventIdx, actionData->sendEmail.message, emailParam.message, MAX_EMAIL_MESSAGE_WIDTH, sizeof(emailParam.message)))
    {
        EPRINT(EVENT_HANDLER, "fail to prepare email msg");
        return FAIL;
    }

    if (ProcessEmail(&emailParam, NULL) != CMD_SUCCESS)
    {
        EPRINT(EVENT_HANDLER, "fail to send email");
        return FAIL;
    }

    DPRINT(EVENT_HANDLER, "email sending success");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sends tcp notification
 * @param   actionData
 * @param   activeDeactive
 * @param   eventIdx
 * @return
 */
static BOOL tcpNotify(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx)
{
    CHAR sendTcp[MAX_TCP_MESSAGE_WIDTH] = { '\0' };

    if (activeDeactive != ACTIVE)
    {
        return FAIL;
    }

    if (FAIL == prepareNotifyMsg(eventIdx, actionData->sendTcp, sendTcp, MAX_TCP_MESSAGE_WIDTH, sizeof(sendTcp)))
    {
        EPRINT(EVENT_HANDLER, "fail to prepare tcp msg");
        return FAIL;
    }

    if(SendTcpNotification(sendTcp) == FAIL)
    {
        EPRINT(EVENT_HANDLER, "fail to send tcp msg");
        return FAIL;
    }

    DPRINT(EVENT_HANDLER, "tcp msg sending success");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sends sms to given mobile number
 * @param   actionData
 * @param   activeDeactive
 * @param   eventIdx
 * @return
 */
static BOOL smsNotify(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx)
{
    SMS_PARAMTER_t  smsParameter;

    if (activeDeactive != ACTIVE)
    {
        return FAIL;
    }

    memset(&smsParameter, 0, sizeof(SMS_PARAMTER_t));
    snprintf(smsParameter.mobileNumber1, MAX_MOBILE_NUMBER_WIDTH, "%s", actionData->smsParameter.mobileNumber1);
    snprintf(smsParameter.mobileNumber2, MAX_MOBILE_NUMBER_WIDTH, "%s", actionData->smsParameter.mobileNumber2);

    if (FAIL == prepareNotifyMsg(eventIdx, actionData->smsParameter.message, smsParameter.message, MAX_SMS_WIDTH, sizeof(smsParameter.message)))
    {
        EPRINT(EVENT_HANDLER, "fail to prepare sms");
        return FAIL;
    }

    if(SendSmsNotification(&smsParameter) == FAIL)
    {
        EPRINT(EVENT_HANDLER, "fail to send sms");
        return FAIL;
    }

    DPRINT(EVENT_HANDLER, "sms sending success");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This sets camera ptz to preset position.
 * @param   actionData
 * @param   activeDeactive
 * @param   eventIdx
 * @return
 */
static BOOL ptzPreset(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx)
{
    UINT8   camIndex;
    CHAR    detail[MAX_EVENT_DETAIL_SIZE];
    CHAR    advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    /* We may get camera number 0 when we change the event and action configuration */
    if ((activeDeactive != ACTIVE) || (actionData->gotoPosition.cameraNumber == 0))
    {
        return FAIL;
    }

    camIndex = GET_CAMERA_INDEX(actionData->gotoPosition.cameraNumber);
    DPRINT(EVENT_HANDLER, "ptz preset set: [camera=%d]", camIndex);
    if (GotoPtzPosition(camIndex, actionData->gotoPosition.presetPosition, NULL, INVALID_CONNECTION, FALSE) == CMD_SUCCESS)
    {
       snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(camIndex));
       snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%d", actionData->gotoPosition.presetPosition);
       WriteEvent(LOG_CAMERA_EVENT, LOG_PRESET_POSITION_CHANGE, detail, advncDetail, EVENT_ALERT);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function activate or de activate system alarm output.
 * @param   actionData
 * @param   activeDeactive
 * @param   eventIdx
 * @return
 */
static BOOL systemAlrmOutput(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx)
{
    UINT8 alarmCnt;

    // check for each system alarm whether to activate or not
    for(alarmCnt = 0; alarmCnt < MAX_ALARM; alarmCnt++)
    {
        if (actionData->systemAlarmOutput[alarmCnt] != ON)
        {
            continue;
        }

        DPRINT(EVENT_HANDLER, "set system alarm: [alarm=%d], [status=%s]", alarmCnt, evtStatusStr[activeDeactive]);
        ChangeAlarmStatus(alarmCnt, activeDeactive);
        MUTEX_LOCK(actionState[eventIdx].actionMutex);
        actionState[eventIdx].sysAlrmOutput[alarmCnt] = activeDeactive;
        MUTEX_UNLOCK(actionState[eventIdx].actionMutex);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function activate or de activate camera alarm output.
 * @param   actionData
 * @param   activeDeactive
 * @param   eventIdx
 * @return
 */
static BOOL cameraAlrmOuput(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx)
{
    UINT8						camAlarmCnt, camIndex;
    CAMERA_CAPABILTY_INFO_t 	camCapability;

    /* We may get camera number 0 when we change the event and action configuration */
    if (actionData->cameraAlarmOutput.cameraNumber == 0)
    {
        return FAIL;
    }

    camIndex = GET_CAMERA_INDEX(actionData->cameraAlarmOutput.cameraNumber);
    if(CMD_SUCCESS != GetSupportedCapability(camIndex, &camCapability))
    {
        EPRINT(EVENT_HANDLER, "failed to get supported capabilities: [camera=%d]", camIndex);
        return FAIL;
    }

    for(camAlarmCnt = 0; camAlarmCnt < camCapability.maxAlarmOutput; camAlarmCnt++)
    {
        if (actionData->cameraAlarmOutput.alarm[camAlarmCnt] != ON)
        {
            continue;
        }

        DPRINT(EVENT_HANDLER, "camera alarm output: [camera=%d], [alarm=%d], [evtStatus=%s]", camIndex, camAlarmCnt, evtStatusStr[activeDeactive]);
        CameraAlarmAction(camIndex, camAlarmCnt, activeDeactive, camAlrmCallback);
        MUTEX_LOCK(actionState[eventIdx].actionMutex);
        actionState[eventIdx].camAlrmOutput[camAlarmCnt] = activeDeactive;
        MUTEX_UNLOCK(actionState[eventIdx].actionMutex);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function starts beep sound from system buzzer.
 * @param   actionData
 * @param   activeDeactive
 * @param   eventIdx
 * @return
 */
static BOOL startBeep(ACTION_PARAMETERS_t * actionData, BOOL activeDeactive, UINT16 eventIdx)
{
    DPRINT(EVENT_HANDLER, "buzzer event: [status=%s]", evtStatusStr[activeDeactive]);
    return (SetSystemStatusLed(SYS_TRG_EVT_RESP, activeDeactive));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function call back of camera alarm output. It will gives actual status whether
 *          camera alarm output was activated or de activated.
 * @param   cameraIndex
 * @param   alarmIndex
 * @param   status
 */
static void camAlrmCallback(UINT8 cameraIndex, UINT8 alarmIndex, NET_CMD_STATUS_e status)
{
    if (status != CMD_SUCCESS)
    {
        EPRINT(EVENT_HANDLER, "alarm callback: [camera=%d], [alarmIndex=%d], [status=%d]", cameraIndex, alarmIndex, status);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   checkScheduleEvent
 * @param   data
 * @return
 */
static BOOL checkScheduleEvent(UINT32 data)
{
    MUTEX_LOCK(overlapCondMutex);
    configUpdate = FALSE;
    configEventUpdate = TOTAL_EVENT_ACTION;
    pthread_cond_signal(&overlapCondSignal);
    MUTEX_UNLOCK(overlapCondMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is called from runMonitorAction() thread at regular interval. It will check
 *          for scheduled action against camere/sensor event and take action if required. If action
 *          was not configured in particular time frame then stop camera event polling or else start.
 */
static void checkScheduleStartStopPollEvent(void)
{
    BOOL					alarmRec = NO;
    UINT8					camCnt, alrmRecCnt, camEvntCnt, sensorCnt;
    UINT8					weekSch, weekDay;
    UINT8					preAlrmRecStart[MAX_CAMERA];
    UINT16					eventIdxNo;
    CAMERA_EVENT_CONFIG_t	camCfg;
    SENSOR_EVENT_CONFIG_t	sensorCfg;
    ACTION_BIT_u            *actionBitPtr;
    struct tm 				brokenTime = { 0 };
    CAMERA_CONFIG_t         cameraConfig[MAX_CAMERA];

    /* Read all camera configuration */
    ReadCameraConfig(cameraConfig);

    if(SUCCESS != GetLocalTimeInBrokenTm(&brokenTime))
    {
        EPRINT(EVENT_HANDLER, "failed to get local time in broken");
    }

    memset(preAlrmRecStart, NO, MAX_CAMERA);

    // ---------------------------- Camera Event And Action --------------------------------
    for(camCnt = 0; camCnt < getMaxCameraForCurrentVariant(); camCnt++)
    {
        /* Check camera config status before proceeding */
        if (cameraConfig[camCnt].camera == DISABLE)
        {
            continue;
        }

        // each camera there was 6 event, so check each event
        for(camEvntCnt = 0; camEvntCnt < MAX_CAMERA_EVENT; camEvntCnt++)
        {
            // check action against camera event was enable
            ReadSingleCameraEventConfig(camCnt, camEvntCnt, &camCfg);
            if (camCfg.action != ENABLE)
            {
                continue;
            }

            eventIdxNo = GET_CAMERA_EVENT(camCnt, camEvntCnt);

            if(checkTimeWindow(&brokenTime, &camCfg.weeklySchedule[brokenTime.tm_wday], &alarmRec, &weekSch, &weekDay) == FALSE)
            {
                MUTEX_LOCK(actionState[eventIdxNo].actionMutex);

                if(actionState[eventIdxNo].status == ACTIVE)
                {
                    weekDay = actionState[eventIdxNo].weekDay;
                    weekSch = actionState[eventIdxNo].weekSch;
                    actionState[eventIdxNo].weekDay = INVALID_WEEK_DAY;
                    actionState[eventIdxNo].weekSch = INVALID_WEEK_SCH;
                    MUTEX_UNLOCK(actionState[eventIdxNo].actionMutex);

                    // Week day change
                    if(weekSch == MAX_EVENT_SCHEDULE)
                    {
                        // Stop old action taken and Start new action
                        actionBitPtr = &camCfg.weeklySchedule[weekDay].entireDayAction;
                    }
                    else
                    {
                        // Stop old action taken and Start new action
                        actionBitPtr = &camCfg.weeklySchedule[weekDay].actionControl[weekSch].scheduleAction;
                    }

                    // Update event status to INACTIVE.
                    takeEventAction(INACTIVE, actionBitPtr, &camCfg.actionParam, eventIdxNo, TRUE);
                }
                else
                {
                    MUTEX_UNLOCK(actionState[eventIdxNo].actionMutex);
                }

                DPRINT(EVENT_HANDLER, "stop event poll: [camera=%d], [event=%s], [eventIdx=%d]", camCnt, cameraEventName[camEvntCnt], eventIdxNo);

                // Stop camera event polling is case of action was not configured at that particular time.
                StopCameraEventPoll(camCnt, camEvntCnt);
            }
            else
            {
                MUTEX_LOCK(actionState[eventIdxNo].actionMutex);

                if(actionState[eventIdxNo].status == INACTIVE)
                {
                    actionState[eventIdxNo].weekDay = weekDay;
                    actionState[eventIdxNo].weekSch = weekSch;
                    MUTEX_UNLOCK(actionState[eventIdxNo].actionMutex);
                    DPRINT(EVENT_HANDLER, "start event poll: [camera=%d], [event=%s], [eventIdx=%d], [camEvntCnt=%d]",
                           camCnt, cameraEventName[camEvntCnt], eventIdxNo, camEvntCnt);

                    // Start camera event polling, if previous state was INACTIVE.
                    StartCameraEventPoll(camCnt, camEvntCnt);
                }
                else
                {
                    // Week day change
                    if((actionState[eventIdxNo].weekDay != brokenTime.tm_wday) && (actionState[eventIdxNo].weekDay != INVALID_WEEK_DAY))
                    {
                        if(actionState[eventIdxNo].weekSch == MAX_EVENT_SCHEDULE)
                        {
                            // Stop old action taken and Start new action
                            actionBitPtr = &camCfg.weeklySchedule[actionState[eventIdxNo].weekDay].entireDayAction;
                        }
                        else
                        {
                            // Stop old action taken and Start new action
                            actionBitPtr = &camCfg.weeklySchedule[actionState[eventIdxNo].weekDay].actionControl[actionState[eventIdxNo].weekSch].scheduleAction;
                        }

                        actionState[eventIdxNo].weekDay = weekDay;
                        actionState[eventIdxNo].weekSch = weekSch;
                        MUTEX_UNLOCK(actionState[eventIdxNo].actionMutex);
                        takeEventAction(INACTIVE, actionBitPtr, &camCfg.actionParam, eventIdxNo, TRUE);
                        DPRINT(EVENT_HANDLER, "week day change taking new action: [camera=%d], [event=%s], [eventIdx=%d]",
                               camCnt, cameraEventName[camEvntCnt], eventIdxNo);
                        sleep(2);

                        if(actionState[eventIdxNo].weekSch == MAX_EVENT_SCHEDULE)
                        {
                            // Stop old action taken and Start new action
                            actionBitPtr = &camCfg.weeklySchedule[actionState[eventIdxNo].weekDay].entireDayAction;
                        }
                        else
                        {
                            // Stop old action taken and Start new action
                            actionBitPtr = &camCfg.weeklySchedule[actionState[eventIdxNo].weekDay].actionControl[actionState[eventIdxNo].weekSch].scheduleAction;
                        }

                        if (CONNECTION_FAILURE != camEvntCnt)
                        {
                            takeEventAction(ACTIVE, actionBitPtr, &camCfg.actionParam, eventIdxNo, TRUE);
                        }
                    }
                    else if((actionState[eventIdxNo].weekSch != weekSch) && (actionState[eventIdxNo].weekSch != INVALID_WEEK_SCH))
                    {
                        // Stop old action taken and Start new action
                        actionBitPtr = &camCfg.weeklySchedule[actionState[eventIdxNo].weekDay].actionControl[actionState[eventIdxNo].weekSch].scheduleAction;
                        actionState[eventIdxNo].weekDay = weekDay;
                        actionState[eventIdxNo].weekSch = weekSch;
                        MUTEX_UNLOCK(actionState[eventIdxNo].actionMutex);
                        takeEventAction(INACTIVE, actionBitPtr, &camCfg.actionParam, eventIdxNo, TRUE);
                        DPRINT(EVENT_HANDLER, "week schedule change taking new action: [camera=%d], [event=%s], [eventIdx=%d]",
                               camCnt, cameraEventName[camEvntCnt], eventIdxNo);
                        sleep(2);

                        // Stop old action taken and Start new action
                        actionBitPtr = &camCfg.weeklySchedule[actionState[eventIdxNo].weekDay].actionControl[actionState[eventIdxNo].weekSch].scheduleAction;

                        if(CONNECTION_FAILURE != camEvntCnt)
                        {
                            takeEventAction(ACTIVE, actionBitPtr, &camCfg.actionParam, eventIdxNo, TRUE);
                        }
                    }
                    else
                    {
                        MUTEX_UNLOCK(actionState[eventIdxNo].actionMutex);
                    }
                }

                if(alarmRec == YES)
                {
                    for(alrmRecCnt = 0; alrmRecCnt < getMaxCameraForCurrentVariant(); alrmRecCnt++)
                    {
                        if(camCfg.actionParam.alarmRecord[alrmRecCnt] == ENABLE)
                        {
                            preAlrmRecStart[alrmRecCnt] = alarmRec;
                        }
                    }
                }
            }
        }
    }

    // ---------------------------- Sensor Event And Action --------------------------------
    for(sensorCnt = 0; sensorCnt < MAX_SENSOR; sensorCnt++)
    {
        ReadSingleSensorEventConfig(sensorCnt, &sensorCfg);

        // check action against camera event was enable
        if(sensorCfg.action != ENABLE)
        {
            continue;
        }

        eventIdxNo = GET_SENSOR_EVENT(sensorCnt);

        if(checkTimeWindow(&brokenTime, &sensorCfg.weeklySchedule[brokenTime.tm_wday], &alarmRec, &weekSch, &weekDay) == FAIL)
        {
            MUTEX_LOCK(actionState[eventIdxNo].actionMutex);

            if(actionState[eventIdxNo].status == ACTIVE)
            {
                weekDay = actionState[eventIdxNo].weekDay;
                weekSch = actionState[eventIdxNo].weekSch;
                actionState[eventIdxNo].weekDay = INVALID_WEEK_DAY;
                actionState[eventIdxNo].weekSch = INVALID_WEEK_SCH;
                MUTEX_UNLOCK(actionState[eventIdxNo].actionMutex);

                // Week day change
                if(weekSch == MAX_EVENT_SCHEDULE)
                {
                    // Stop old action taken and Start new action
                    actionBitPtr = &sensorCfg.weeklySchedule [weekDay].entireDayAction;
                }
                else
                {
                    // Stop old action taken and Start new action
                    actionBitPtr = &sensorCfg.weeklySchedule[weekDay].actionControl[weekSch].scheduleAction;
                }

                takeEventAction(INACTIVE, actionBitPtr, &sensorCfg.actionParam, eventIdxNo, TRUE);
            }
            else
            {
                MUTEX_UNLOCK(actionState[eventIdxNo].actionMutex);
            }
            DPRINT(EVENT_HANDLER, "stop sensor poll: [sensor=%d], [eventIdx=%d]", sensorCnt, eventIdxNo);
        }
        else
        {
            MUTEX_LOCK(actionState[eventIdxNo].actionMutex);

            if(actionState[eventIdxNo].status == INACTIVE)
            {
                actionState[eventIdxNo].weekDay = weekDay;
                actionState[eventIdxNo].weekSch = weekSch;
                MUTEX_UNLOCK(actionState[eventIdxNo].actionMutex);

                if(GetSensorStatus(sensorCnt, 0) == ACTIVE)
                {
                    SensorEventNotify(sensorCnt, ACTIVE);
                }
            }
            else
            {
                // Week day change
                if((actionState[eventIdxNo].weekDay != brokenTime.tm_wday) && (actionState[eventIdxNo].weekDay != INVALID_WEEK_DAY))
                {
                    if(actionState[eventIdxNo].weekSch == MAX_EVENT_SCHEDULE)
                    {
                        // Stop old action taken and Start new action
                        actionBitPtr = &sensorCfg.weeklySchedule[actionState[eventIdxNo].weekDay].entireDayAction;
                    }
                    else
                    {
                        // Stop old action taken and Start new action
                        actionBitPtr = &sensorCfg.weeklySchedule[actionState[eventIdxNo].weekDay].actionControl[actionState[eventIdxNo].weekSch].scheduleAction;
                    }

                    actionState[eventIdxNo].weekDay = weekDay;
                    actionState[eventIdxNo].weekSch = weekSch;
                    MUTEX_UNLOCK(actionState[eventIdxNo].actionMutex);
                    takeEventAction(INACTIVE, actionBitPtr, &sensorCfg.actionParam, eventIdxNo, TRUE);
                    DPRINT(EVENT_HANDLER, "weekday change, stop sensor poll: [sensor=%d], [eventIdx=%d]", sensorCnt, eventIdxNo);
                    sleep(2);

                    if(actionState[eventIdxNo].weekSch == MAX_EVENT_SCHEDULE)
                    {
                        // Stop old action taken and Start new action
                        actionBitPtr = &camCfg.weeklySchedule[actionState[eventIdxNo].weekDay].entireDayAction;
                    }
                    else
                    {
                        // Stop old action taken and Start new action
                        actionBitPtr = &camCfg.weeklySchedule[actionState[eventIdxNo].weekDay].actionControl[actionState[eventIdxNo].weekSch].scheduleAction;
                    }

                    takeEventAction(ACTIVE, actionBitPtr, &camCfg.actionParam, eventIdxNo, TRUE);
                }
                else if((actionState[eventIdxNo].weekSch != weekSch) && (actionState[eventIdxNo].weekSch != INVALID_WEEK_SCH))
                {
                    // Stop old action taken and Start new action
                    actionBitPtr = &sensorCfg.weeklySchedule[actionState[eventIdxNo].weekDay].actionControl[actionState[eventIdxNo].weekSch].scheduleAction;

                    actionState[eventIdxNo].weekDay = weekDay;
                    actionState[eventIdxNo].weekSch = weekSch;
                    MUTEX_UNLOCK(actionState[eventIdxNo].actionMutex);
                    takeEventAction(INACTIVE, actionBitPtr, &camCfg.actionParam, eventIdxNo, TRUE);
                    DPRINT(EVENT_HANDLER, "weekday schedule change taking new action sensor poll: [sensor=%d], [eventIdx=%d]", sensorCnt, eventIdxNo);
                    sleep(2);

                    // Stop old action taken and Start new action
                    actionBitPtr = &camCfg.weeklySchedule[actionState[eventIdxNo].weekDay].actionControl[actionState[eventIdxNo].weekSch].scheduleAction;

                    takeEventAction(ACTIVE, actionBitPtr, &camCfg.actionParam, eventIdxNo, TRUE);
                }
                else
                {
                    MUTEX_UNLOCK(actionState[eventIdxNo].actionMutex);
                }
            }

            if(alarmRec == YES)
            {
                for(alrmRecCnt = 0; alrmRecCnt < getMaxCameraForCurrentVariant(); alrmRecCnt++)
                {
                    if(sensorCfg.actionParam.alarmRecord[alrmRecCnt] == ENABLE)
                    {
                        preAlrmRecStart[alrmRecCnt] = alarmRec;
                    }
                }
            }
        }
    }

    for(camCnt = 0; camCnt < getMaxCameraForCurrentVariant(); camCnt++)
    {
        if(preAlrmRecStart[camCnt] == YES)
        {
            EntPreAlrmRecStrm(camCnt);
        }
        else
        {
            ExitPreAlrmRecStrm(camCnt);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   takeEventAction
 * @param   eventState
 * @param   actionBitField
 * @param   actionParam
 * @param   eventIndex
 * @param   updateStatus
 */
static void takeEventAction(BOOL eventState, ACTION_BIT_u * actionBitField, ACTION_PARAMETERS_t * actionParam, UINT16 eventIndex, BOOL updateStatus)
{
    DPRINT(EVENT_HANDLER, "start event action: [eventIdx=%d], [state=%d], [actionBits=0x%X]", eventIndex, eventState, actionBitField->actionBitGroup);
    if (updateStatus == TRUE)
    {
        MUTEX_LOCK(actionState[eventIndex].actionMutex);
        actionState[eventIndex].status = eventState;
        MUTEX_UNLOCK(actionState[eventIndex].actionMutex);
    }

    MUTEX_LOCK(actionState[eventIndex].actionThread.threadMutex);
    if (actionState[eventIndex].actionThread.threadStatus == INACTIVE)
    {
        actionState[eventIndex].actionThread.threadStatus = ACTIVE;
        MUTEX_UNLOCK(actionState[eventIndex].actionThread.threadMutex);

        actionState[eventIndex].actionThread.eventIdx = eventIndex;
        actionState[eventIndex].actionThread.eventStatus = eventState;
        memcpy(&actionState[eventIndex].actionThread.actionBitField, actionBitField, sizeof(ACTION_BIT_u));
        memcpy(&actionState[eventIndex].actionThread.actionParam, actionParam, sizeof(ACTION_PARAMETERS_t));

        if (FALSE == Utils_CreateThread(NULL, doEventAction, &actionState[eventIndex].actionThread, DETACHED_THREAD, DO_EVENT_ACTION_STACK_SZ))
        {
            MUTEX_LOCK(actionState[eventIndex].actionThread.threadMutex);
            actionState[eventIndex].actionThread.threadStatus = INACTIVE;
            MUTEX_UNLOCK(actionState[eventIndex].actionThread.threadMutex);
            EPRINT(EVENT_HANDLER, "fail to create action thread: [status=%s], [eventIdx=%d]", evtStatusStr[eventState], eventIndex);
        }
        else
        {
            DPRINT(EVENT_HANDLER, "action thread created: [status=%s], [eventIdx=%d]", evtStatusStr[eventState], eventIndex);
        }
    }
    else
    {
        MUTEX_UNLOCK(actionState[eventIndex].actionThread.threadMutex);
        EPRINT(EVENT_HANDLER, "action thread already running: [status=%s], [eventIdx=%d]", evtStatusStr[eventState], eventIndex);
    }

    MUTEX_LOCK(actionScheduleOverlap[eventIndex].runMoniterMutex);
    if (updateStatus == TRUE)
    {
        memcpy(&actionScheduleOverlap[eventIndex].oldActionParam, actionParam, sizeof(ACTION_PARAMETERS_t));
    }
    MUTEX_UNLOCK(actionScheduleOverlap[eventIndex].runMoniterMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   doEventAction
 * @param   threadArg
 * @return
 */
static VOIDPTR doEventAction(VOIDPTR threadArg)
{
    EVENT_ACTION_e  evtAct;
    ACTION_THREAD_t *actionThreadPtr = (ACTION_THREAD_t *)threadArg;

    for (evtAct = EVNT_START_BEEP; evtAct < MAX_EVENT_ACTION; evtAct++)
    {
        if (evtAct == EVNT_VIDEO_POPUP)
        {
            continue;
        }

        if (GET_BIT(actionThreadPtr->actionBitField.actionBitGroup, evtAct) == TRUE)
        {
            if (NULL != actionTaken[evtAct])
            {
                actionTaken[evtAct](&actionThreadPtr->actionParam, actionThreadPtr->eventStatus, actionThreadPtr->eventIdx);
            }
        }
    }

    MUTEX_LOCK(actionThreadPtr->threadMutex);
    actionThreadPtr->threadStatus = INACTIVE;
    MUTEX_UNLOCK(actionThreadPtr->threadMutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function get status sensor event notify
 * @param   sysEvent
 * @return
 */
static BOOL getSystemEventStatus(UINT8 sysEvent)
{
    MUTEX_LOCK(systemEvtStatus[sysEvent].threadMutex);
    BOOL action = systemEvtStatus[sysEvent].status;
    MUTEX_UNLOCK(systemEvtStatus[sysEvent].threadMutex);
    return action;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function take schedule backup fail action  event notify
 * @param   action
 */
void TakeScheduleBackupFailSystemAction(UINT8 action)
{
    systemEventNofity(SYS_EVT_SCHEDULE_BACKUP_FAIL, action);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This thread monitor change in event and action configuration.
 * @param   threadArg
 * @return
 */
static VOIDPTR runMonitorAction(VOIDPTR threadArg)
{
    UINT16 					eventIdxNo = 0;
    UINT8 					camCnt = 0;
    UINT8					sensorCnt = 0;
    UINT8 					camEventCnt = 0;
    CAMERA_EVENT_CONFIG_t	camEventCfg;
    SENSOR_EVENT_CONFIG_t	sensorEventCfg;
    struct tm 				newBrokenTime = { 0 };
    ACTION_BIT_u			localBitFieldOld;
    ACTION_PARAMETERS_t		oldLocalActionParam;
    ACTION_PARAMETERS_t		newLocalActionParam;
    BOOL					configCheckStatus = FALSE;
    UINT32					configEventIndex = TOTAL_EVENT_ACTION;

    THREAD_START("EVENT_MONITOR");

    while(TRUE)
    {
        MUTEX_LOCK(overlapCondMutex);
        pthread_cond_wait(&overlapCondSignal, &overlapCondMutex);
        configCheckStatus = configUpdate;
        configEventIndex = configEventUpdate;
        configEventUpdate = TOTAL_EVENT_ACTION;
        configUpdate = FALSE;
        MUTEX_UNLOCK(overlapCondMutex);

        checkScheduleStartStopPollEvent();

        if(SUCCESS != GetLocalTimeInBrokenTm(&newBrokenTime))
        {
            EPRINT(EVENT_HANDLER, "failed to get local time in broken");
        }

        // ---------------------------- Camera Event And Action --------------------------------
        for(camCnt = 0; camCnt < getMaxCameraForCurrentVariant(); camCnt++)
        {
            // each camera there was 6 event, so check each event
            for(camEventCnt = 0; camEventCnt < MAX_CAMERA_EVENT; camEventCnt++)
            {
                ReadSingleCameraEventConfig(camCnt, camEventCnt, &camEventCfg);
                eventIdxNo = GET_CAMERA_EVENT(camCnt, camEventCnt);

                // check action against camera event was enable
                if(camEventCfg.action != ENABLE)
                {
                    continue;
                }

                CheckEventTimeWindow(&camEventCfg.weeklySchedule[newBrokenTime.tm_wday], &actionScheduleOverlap[eventIdxNo].actionBitFieldNew, &newBrokenTime);

                if((memcmp(&actionScheduleOverlap[eventIdxNo].actionBitFieldOld, &actionScheduleOverlap[eventIdxNo].actionBitFieldNew,
                           sizeof(ACTION_BIT_u)) != STATUS_OK) || ((configCheckStatus == TRUE) && (configEventIndex == eventIdxNo)))
                {
                    DPRINT(EVENT_HANDLER, "take new camera action as per event: [camera=%d], [event=%s], [eventIdx=%d]",
                           camCnt, cameraEventName[camEventCnt], eventIdxNo);
                    memcpy(&localBitFieldOld, &actionScheduleOverlap[eventIdxNo].actionBitFieldOld, sizeof(ACTION_BIT_u));
                    memcpy(&actionScheduleOverlap[eventIdxNo].actionBitFieldOld, &actionScheduleOverlap[eventIdxNo].actionBitFieldNew, sizeof(ACTION_BIT_u));

                    MUTEX_LOCK(actionScheduleOverlap[eventIdxNo].runMoniterMutex);
                    memcpy(&actionScheduleOverlap[eventIdxNo].newActionParam, &camEventCfg.actionParam, sizeof(ACTION_PARAMETERS_t));
                    memcpy(&newLocalActionParam, &actionScheduleOverlap[eventIdxNo].newActionParam, sizeof(ACTION_PARAMETERS_t));
                    memcpy(&oldLocalActionParam, &actionScheduleOverlap[eventIdxNo].oldActionParam, sizeof(ACTION_PARAMETERS_t));
                    memcpy(&actionScheduleOverlap[eventIdxNo].oldActionParam, &actionScheduleOverlap[eventIdxNo].newActionParam, sizeof(ACTION_PARAMETERS_t));
                    MUTEX_UNLOCK(actionScheduleOverlap[eventIdxNo].runMoniterMutex);

                    applyConfigChanges(eventIdxNo, &oldLocalActionParam, &newLocalActionParam,
                                       &localBitFieldOld, &actionScheduleOverlap[eventIdxNo].actionBitFieldNew);
                }
            }
        }

        // ---------------------------- Sensor Event And Action --------------------------------
        for(sensorCnt = 0; sensorCnt < MAX_SENSOR; sensorCnt++)
        {
            ReadSingleSensorEventConfig(sensorCnt, &sensorEventCfg);
            eventIdxNo = GET_SENSOR_EVENT(sensorCnt);

            // check action against camera event was enable
            if(sensorEventCfg.action != ENABLE)
            {
                continue;
            }

            CheckEventTimeWindow(&sensorEventCfg.weeklySchedule[newBrokenTime.tm_wday], &actionScheduleOverlap[eventIdxNo].actionBitFieldNew, &newBrokenTime);

            if((memcmp(&actionScheduleOverlap[eventIdxNo].actionBitFieldOld, &actionScheduleOverlap[eventIdxNo].actionBitFieldNew,
                       sizeof(ACTION_BIT_u)) != STATUS_OK) || ((configCheckStatus == TRUE) && (configEventIndex == eventIdxNo)))
            {
                DPRINT(EVENT_HANDLER, "take new sensor action as per event: [camera=%d], [eventIdx=%d]", camCnt, eventIdxNo);
                memcpy(&localBitFieldOld, &actionScheduleOverlap[eventIdxNo].actionBitFieldOld, sizeof(ACTION_BIT_u));
                memcpy(&actionScheduleOverlap[eventIdxNo].actionBitFieldOld, &actionScheduleOverlap[eventIdxNo].actionBitFieldNew, sizeof(ACTION_BIT_u));

                MUTEX_LOCK(actionScheduleOverlap[eventIdxNo].runMoniterMutex);
                memcpy(&actionScheduleOverlap[eventIdxNo].newActionParam, &sensorEventCfg.actionParam, sizeof(ACTION_PARAMETERS_t));
                memcpy(&newLocalActionParam, &actionScheduleOverlap[eventIdxNo].newActionParam, sizeof(ACTION_PARAMETERS_t));
                memcpy(&oldLocalActionParam, &actionScheduleOverlap[eventIdxNo].oldActionParam, sizeof(ACTION_PARAMETERS_t));
                memcpy(&actionScheduleOverlap[eventIdxNo].oldActionParam, &actionScheduleOverlap[eventIdxNo].newActionParam, sizeof(ACTION_PARAMETERS_t));
                MUTEX_UNLOCK(actionScheduleOverlap[eventIdxNo].runMoniterMutex);

                applyConfigChanges(eventIdxNo, &oldLocalActionParam, &newLocalActionParam,
                                   &localBitFieldOld, &actionScheduleOverlap[eventIdxNo].actionBitFieldNew);
            }
        }
    }

    pthread_exit(NULL);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
