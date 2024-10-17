//#################################################################################################
// @FILE BRIEF
//#################################################################################################
/**
@file   RecordManager.c
@brief  Record Manger handles the recording of the particular channel also creates the disk manager
        and Camera Interface session
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/resource.h>

/* Application Includes */
#include "RecordManager.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define TWO_FRAME_TIME_DIFF			(-30)
#define RECORD_SCHEDULER			(10)
#define MAX_REC_MONITORING_CNT		(10)
#define RECORD_INIT_TIME	 		(60)

/* Use Default Stack Size*/
#define WRITE_FRAME_THREAD_STACK_SZ (0*MEGA_BYTE)

#define STATE_OVERRIDE_PRINT(camera, oldStateStr, newStateStr) \
    WPRINT(RECORD_MANAGER, "state override: [camera=%d], [old=%s], [new=%s]", camera, oldStateStr, newStateStr)

//#################################################################################################
// @DATA_TYPES
//#################################################################################################
typedef enum
{
    RECORD_OFF = 0,
    RECORD_OFF_WAIT,
    RECORD_RESTART_CLEANUP,
    RECORD_RESTART_INIT,
    RECORD_ON_WAIT,
    RECORD_ON,
    RECORD_DRIVE_SWITCH,
    RECORD_STREAM_SWITCH,
    RECORD_STATE_MAX

}RECORD_STATE_e;

typedef enum
{
    REC_STOP = 0,
    REC_START,
    REC_FAIL,
    MAX_REC_STATE

}RCD_STATE_e;

typedef enum
{
    EVNT_REC_STOP_VIDEO_LOSS,
    EVNT_REC_STOP_DISK_FULL,
    EVNT_REC_STOP_MEDIA_BUSY,
    EVNT_REC_STOP_DISK_FAULT,
    EVNT_REC_STOP_NDD1_DISCONNECT,
    EVNT_REC_STOP_NDD2_DISCONNECT,
    EVNT_REC_STOP_HDD_GROUP_CHANGED,
    MAX_EVNT_REC_STOP

}LOG_REC_STOP_e;

// Information of all recording session
typedef struct
{
    /* This field was set whenever request for enter to pre alarm record mode. Then if other module calls StartRecord
     * then this module was check for this field if it was set then no need to start the record. Now for StopRecord it
     * was check for this field if set then no need to stop record stream. If pre alarm time was over and request for
     * exit from pre alarm record then this module checks for any recording was still on goining then it will not stop
     * record stream but it will clear this field status. */
    BOOL				preAlrmRcrdStrm;	// TRUE or FALSE

    BOOL				preCosecRcrdStrm;
    RECORD_STATE_e		recordStatus;
    BOOL				preRecdStrmSkip;

    // alarm record count
    UINT8				alrmRecordCnt;

    // record type for which session was started
    UINT8				recordType;
    VIDEO_TYPE_e		streamType;
    LocalTime_t			localTime;

    CHAR				cosecRecUserName[MAX_EVENT_ADVANCE_DETAIL_SIZE];
    CHAR				manualRecUserName[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    LOG_REC_STOP_e		recordFailReason;
    BOOL				adaptiveRecF;

    RCD_STATE_e			recordHeathState[MAX_RECORD];
    pthread_mutex_t		dataMutex;

}RECORD_SESSION_t;

//#################################################################################################
// @FUNCTION PROTOTYPE
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void initRecSessionData(UINT8 channelCnt);
//-------------------------------------------------------------------------------------------------
static void entPreAlrmCallback(const CI_STREAM_RESP_PARAM_t *respParam);
//-------------------------------------------------------------------------------------------------
static BOOL exitPreCosecRecStrm(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
static BOOL entPreCosecRecStrm(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
static void entPreCosecCallback(const CI_STREAM_RESP_PARAM_t *respParam);
//-------------------------------------------------------------------------------------------------
static void startStreamCallback(const CI_STREAM_RESP_PARAM_t *respParam);
//-------------------------------------------------------------------------------------------------
static void writeRecordingEvent(UINT8 channelNo, UINT8 recType, RCD_STATE_e action, LOG_REC_STOP_e eventDetailType);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e startInternalRecord(UINT8 channelNo, RECORD_TYPE_e recType, UINT32 recDuration);
//-------------------------------------------------------------------------------------------------
static BOOL stopInternalRecord(UINT8 channelNo, RECORD_TYPE_e recordType, BOOL forceStop);
//-------------------------------------------------------------------------------------------------
static void postAlarmRecCallback(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void stopCosecRec(UINT32 data);
//-------------------------------------------------------------------------------------------------
static VOIDPTR writeFrame(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static void checkTimeSlotForRecording(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL valSchdlRecordTime(UINT8 channelNo, struct tm *brokenTime);
//-------------------------------------------------------------------------------------------------
static BOOL valAdaptiveRecordTime(UINT8 channelNo, struct tm *brokenTime);
//-------------------------------------------------------------------------------------------------
static void recordInitHandling(UINT32 data);
//-------------------------------------------------------------------------------------------------
static UINT8 getRecordChannelNo(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
static UINT8 getPrevStreamRecordChannelNo(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLE
//#################################################################################################
// Record Session
static RECORD_SESSION_t 			recordSession[MAX_CAMERA];

// Stop recorder thread indicator
static BOOL 						recTerminateFlg;

// Conditional mutex for recorder thread
/** Note below mutex is recursive in nature , so it is possible to take lock over a lock */
static pthread_mutex_t 				frameWrCondMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

// Conditional signal for recorder thread
static pthread_cond_t 				frameWrCondSignal;

// Recorder thread ID
static pthread_t 					recThreadId;

static BOOL							isRecInitDone = FALSE;
static BOOL                         recordWriterStatus[MAX_CAMERA];
static BOOL                         recordRestartWaitStatus[MAX_CAMERA];
static BOOL                         recordOnWaitStatus[MAX_CAMERA];
static BOOL                         recordOffWaitStatus[MAX_CAMERA];
static TIMER_HANDLE					recTimerHandle = INVALID_TIMER_HANDLE;
static TIMER_HANDLE					recInitHandle = INVALID_TIMER_HANDLE;
static TIMER_HANDLE					postAlarmRecTimerHandle[MAX_CAMERA];
static TIMER_HANDLE					postCosecRecTimerHandle[MAX_CAMERA];

static const CHARPTR recTypeStr[MAX_RECORD] = {"Manual", "Alarm", "Schedule", "Cosec"};

static CHARPTR recordStateStr[RECORD_STATE_MAX] =
{
    "RECORD_OFF",
    "RECORD_OFF_WAIT",
    "RECORD_RESTART_CLEANUP",
    "RECORD_RESTART_INIT",
    "RECORD_ON_WAIT",
    "RECORD_ON",
    "RECORD_DRIVE_SWITCH",
    "RECORD_STREAM_SWITCH"
};

static CHARPTR recordStopStateStr[MAX_EVNT_REC_STOP] =
{
    "Video Loss",
    "Disk Full",
    "Recording Media Busy",
    "Disk Fault",
    "NDD1 Disconnected",
    "NDD2 Disconnected",
    "HDD Group Changed",
};

//#################################################################################################
// @FUNCTION DEFINATIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes all session of disk manager to invalid and also create a thread
 *          for record media frame.
 * @return  True on success else false is return
 */
BOOL InitRecordManager(void)
{
    UINT8           channelCnt;
    UINT8           recType;
    TIMER_INFO_t    timerInfo;

    /* Init of all session of record manager */
    for (channelCnt = 0; channelCnt < MAX_CAMERA; channelCnt++)
    {
        MUTEX_INIT(recordSession[channelCnt].dataMutex, NULL);
        initRecSessionData(channelCnt);
        recordSession[channelCnt].recordFailReason = MAX_EVNT_REC_STOP;
        postAlarmRecTimerHandle[channelCnt] = INVALID_TIMER_HANDLE;
        postCosecRecTimerHandle[channelCnt] = INVALID_TIMER_HANDLE;
        recordWriterStatus[channelCnt] = FALSE;
        recordRestartWaitStatus[channelCnt] = FALSE;
        recordOnWaitStatus[channelCnt] = FALSE;
        recordOffWaitStatus[channelCnt] = FALSE;
        for(recType = 0; recType < MAX_RECORD; recType++)
        {
            recordSession[channelCnt].recordHeathState[recType] = REC_STOP;
        }
    }

    recTerminateFlg = FALSE;
    pthread_cond_init(&frameWrCondSignal, NULL);

    /* Registered for schedule recording to execute at every 10 sec */
    timerInfo.funcPtr = checkTimeSlotForRecording;
    timerInfo.data = 0;
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(RECORD_SCHEDULER);

    if(StartTimer(timerInfo, &recTimerHandle) == SUCCESS)
    {
        if (FAIL == Utils_CreateThread(&recThreadId, &writeFrame, NULL, JOINABLE_THREAD, WRITE_FRAME_THREAD_STACK_SZ))
        {
            recTerminateFlg = TRUE;
            EPRINT(RECORD_MANAGER, "fail to create recorder thread");
        }
    }
    else
    {
        EPRINT(RECORD_MANAGER, "fail to start record time slot check timer");
    }

    /* RECORD_INIT_TIME is time to block the record fail event from first buzzer start */
    isRecInitDone = FALSE;
    timerInfo.funcPtr = recordInitHandling;
    timerInfo.data = 0;
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(RECORD_INIT_TIME);

    if(StartTimer(timerInfo, &recInitHandle) == FAIL)
    {
        EPRINT(RECORD_MANAGER, "fail to start record init timer");
    }

    return SUCCESS;
}
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function de initializes all resources used by record manager module.
 * @return  True on success else false is return
 */
BOOL DeInitRecordManager(void)
{
    UINT8 channelCnt;

    /* closed all recording session */
    for (channelCnt = 0; channelCnt < getMaxCameraForCurrentVariant(); channelCnt++)
    {
        if(recordSession[channelCnt].recordStatus != RECORD_ON)
        {
            continue;
        }

        StopStream(getRecordChannelNo(channelCnt), CI_STREAM_CLIENT_RECORD);
    }

    DeleteTimer(&recTimerHandle);

    /* No need to write any frames into file because system goes to restart */
    MUTEX_LOCK(frameWrCondMutex);
    for (channelCnt = 0; channelCnt < getMaxCameraForCurrentVariant(); channelCnt++)
    {
        recordWriterStatus[channelCnt] = FALSE;
        recordRestartWaitStatus[channelCnt] = FALSE;
        recordOnWaitStatus[channelCnt] = FALSE;
        recordOffWaitStatus[channelCnt] = FALSE;
    }
    recTerminateFlg = TRUE;
    pthread_cond_signal(&frameWrCondSignal);
    MUTEX_UNLOCK(frameWrCondMutex);

    pthread_join(recThreadId, NULL);
    DPRINT(SYS_LOG, "record manager deinit successfully");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function de initializes all resources used by record manager module.
 * @param   channelCnt - RecordManager session channel no.
 */
static void initRecSessionData(UINT8 channelCnt)
{
    recordSession[channelCnt].recordStatus = RECORD_OFF;
    recordSession[channelCnt].recordType = 0;
    recordSession[channelCnt].localTime.mSec = 0;
    recordSession[channelCnt].localTime.totalSec = 0;
    recordSession[channelCnt].preAlrmRcrdStrm = FALSE;
    recordSession[channelCnt].preRecdStrmSkip = NO;
    recordSession[channelCnt].alrmRecordCnt = 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   RmConfigChangeNotify
 * @param   newManualRecordConfig
 * @param   oldManualRecordConfig
 * @return
 */
void RmConfigChangeNotify(MANUAL_RECORD_CONFIG_t newManualRecordConfig, MANUAL_RECORD_CONFIG_t *oldManualRecordConfig, UINT8 cameraIndex)
{
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    if(newManualRecordConfig.manualRecordStatus == oldManualRecordConfig->manualRecordStatus)
    {
        return;
    }

    if(newManualRecordConfig.manualRecordStatus == DISABLE)
    {
        stopInternalRecord(cameraIndex, MANUAL_RECORD, FALSE);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was used to start the recording stream for pre alarm record duration. This
 *          function was request for start the record stream to camera interface. This function starts
 *          the record stream if recording was not going on. If recording was going on then this
 *          function only registers the pre alarm recording was started from external module.
 * @param   channelNo - RecordManager channel no.
 * @return  SUCCESS/FAIL
 */
BOOL EntPreAlrmRecStrm(UINT8 channelNo)
{
    ALARM_RECORD_CONFIG_t alarmRecordCfg;

    // Validate input channel Number
    if (channelNo >= getMaxCameraForCurrentVariant())
    {
        return FAIL;
    }

    ReadSingleAlarmRecordConfig(channelNo, &alarmRecordCfg);
    MUTEX_LOCK(frameWrCondMutex);
    MUTEX_LOCK(recordSession[channelNo].dataMutex);

    if((recordSession[channelNo].preAlrmRcrdStrm == FALSE) && (alarmRecordCfg.preRecordTime > 0))
    {
        recordSession[channelNo].preAlrmRcrdStrm = TRUE;
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

        if (StartStream(getRecordChannelNo(channelNo), entPreAlrmCallback, CI_STREAM_CLIENT_PRE_ALARM_RECORD) == CMD_SUCCESS)
        {
            DPRINT(RECORD_MANAGER, "enter pre-alarm record: [camera=%d]", channelNo);
        }
        else
        {
            MUTEX_LOCK(recordSession[channelNo].dataMutex);
            recordSession[channelNo].preAlrmRcrdStrm = FALSE;
            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
        }
    }
    else
    {
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
    }

    MUTEX_UNLOCK(frameWrCondMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was used to start the recording stream for pre COSEC record duration. This
 *          function was request for start the record stream to camera interface. This function starts
 *          the record stream if recording was not going on. If recording was going on then this
 *          function only registers the pre alarm recording was started from external module.
 * @param   channelNo - Channel no.
 * @return  SUCCESS/FAIL
 */
static BOOL entPreCosecRecStrm(UINT8 channelNo)
{
    // Validate input channel Number
    if (channelNo >= getMaxCameraForCurrentVariant())
    {
        return FAIL;
    }

    MUTEX_LOCK(frameWrCondMutex);
    MUTEX_LOCK(recordSession[channelNo].dataMutex);

    if(recordSession[channelNo].preCosecRcrdStrm == FALSE)
    {
        recordSession[channelNo].preCosecRcrdStrm = TRUE;
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

        if (StartStream(getRecordChannelNo(channelNo), entPreCosecCallback, CI_STREAM_CLIENT_COSEC_RECORD) == CMD_SUCCESS)
        {
            DPRINT(RECORD_MANAGER, "enter pre-cosec record: [camera=%d]", channelNo);
        }
        else
        {
            MUTEX_LOCK(recordSession[channelNo].dataMutex);
            recordSession[channelNo].preCosecRcrdStrm = FALSE;
            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
        }
    }
    else
    {
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
    }

    MUTEX_UNLOCK(frameWrCondMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was used to stop the recording stream for pre alarm record duration. This
 *          function stops the record stream if recording was not going on. If recording was going
 *          on then this function only un-registers the pre alarm recording was started from external module.
 * @param   channelNo
 * @return  SUCCESS/FAIL
 */
BOOL ExitPreAlrmRecStrm(UINT8 channelNo)
{
    // Validate input channel Number
    if (channelNo >= getMaxCameraForCurrentVariant())
    {
        return FAIL;
    }

    MUTEX_LOCK(frameWrCondMutex);
    MUTEX_LOCK(recordSession[channelNo].dataMutex);

    if(recordSession[channelNo].preAlrmRcrdStrm == TRUE)
    {
        recordSession[channelNo].preAlrmRcrdStrm = FALSE;
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

        StopStream(getRecordChannelNo(channelNo), CI_STREAM_CLIENT_PRE_ALARM_RECORD);
        DPRINT(RECORD_MANAGER, "exit pre-alarm record: [camera=%d]", channelNo);
    }
    else
    {
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
    }

    MUTEX_UNLOCK(frameWrCondMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was used to stop the recording stream for pre Cosec record duration. This
 *          function stops the record stream if recording was not going on. If recording was going
 *          on then this function only un-registers the pre Cosec recording was started from external module.
 * @param   channelNo
 * @return  SUCCESS/FAIL
 */
static BOOL exitPreCosecRecStrm(UINT8 channelNo)
{
    // Validate input channel Number
    if(channelNo >= getMaxCameraForCurrentVariant())
    {
        return FAIL;
    }

    MUTEX_LOCK(frameWrCondMutex);
    MUTEX_LOCK(recordSession[channelNo].dataMutex);

    if(recordSession[channelNo].preCosecRcrdStrm == TRUE)
    {
        recordSession[channelNo].preCosecRcrdStrm = FALSE;
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

        StopStream(getRecordChannelNo(channelNo), CI_STREAM_CLIENT_COSEC_RECORD);
        DPRINT(RECORD_MANAGER, "exit pre-cosec record: [camera=%d]", channelNo);
    }
    else
    {
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
    }

    MUTEX_UNLOCK(frameWrCondMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was used to register the notify function on hard disk event (like formatting
 *          disk). It will start all running recording after hard disk event.
 * @return  SUCCESS/FAIL
 */
BOOL StartRecordOnHddEvent(void)
{
    BOOL sendSingalF = FALSE;
    UINT8 channelNo;

    MUTEX_LOCK(frameWrCondMutex);
    for(channelNo = 0; channelNo < getMaxCameraForCurrentVariant(); channelNo++)
    {
        if(recordSession[channelNo].recordStatus == RECORD_RESTART_CLEANUP)
        {
            if (TRUE == recordWriterStatus[channelNo])
            {
                STATE_OVERRIDE_PRINT(channelNo, recordStateStr[recordSession[channelNo].recordStatus], recordStateStr[RECORD_RESTART_INIT]);
            }

            // recording was going on for this channel then it must be stopped because of hard disk event.
            recordSession[channelNo].recordStatus = RECORD_RESTART_INIT;
            recordWriterStatus[channelNo] = TRUE;
            sendSingalF = TRUE;
            DPRINT(RECORD_MANAGER, "record session state: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_RESTART_INIT]);
        }
        else if(recordSession[channelNo].recordStatus == RECORD_OFF)
        {
            if(recordSession[channelNo].recordHeathState[ALARM_RECORD] == REC_FAIL)
            {
                DPRINT(RECORD_MANAGER, "start alarm record: [camera=%d]", channelNo);
                StartRecord(channelNo, ALARM_RECORD, DEFAULT_REC_DURATION, NULL);
            }
        }
    }

    if (sendSingalF == TRUE)
    {
        pthread_cond_signal(&frameWrCondSignal);
    }
    MUTEX_UNLOCK(frameWrCondMutex);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was used to register the notify function on hard disk event (like formatting
 *          disk). It will stop all running recording because of user has request to format the disk.
 * @return  SUCCESS/FAIL
 */
BOOL StopRecordOnHddEvent(void)
{
    BOOL sendSingalF = FALSE;
    UINT8 channelNo;

    MUTEX_LOCK(frameWrCondMutex);
    for(channelNo = 0; channelNo < getMaxCameraForCurrentVariant(); channelNo++)
    {
        // recording was going on for this channel then it must be stopped because of hard disk event.
        recordSession[channelNo].recordFailReason = EVNT_REC_STOP_MEDIA_BUSY;
        if((recordSession[channelNo].recordStatus == RECORD_ON) || (recordSession[channelNo].recordStatus == RECORD_DRIVE_SWITCH))
        {
            if (TRUE == recordWriterStatus[channelNo])
            {
                STATE_OVERRIDE_PRINT(channelNo, recordStateStr[recordSession[channelNo].recordStatus], recordStateStr[RECORD_RESTART_CLEANUP]);
            }

            recordSession[channelNo].recordStatus = RECORD_RESTART_CLEANUP;
            recordWriterStatus[channelNo] = TRUE;
            sendSingalF = TRUE;
            DPRINT(RECORD_MANAGER, "record session state: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_RESTART_CLEANUP]);
        }
    }

    if (sendSingalF == TRUE)
    {
        pthread_cond_signal(&frameWrCondSignal);
    }

    MUTEX_UNLOCK(frameWrCondMutex);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was used switch current recording disk on disk full.
 * @param   cameraMask
 * @return  SUCCESS/FAIL
 */
BOOL SwitchRecordSession(CAMERA_BIT_MASK_t cameraMask)
{
    BOOL sendSingalF = FALSE;
    UINT8 channelNo;

    MUTEX_LOCK(frameWrCondMutex);
    for(channelNo = 0; channelNo < getMaxCameraForCurrentVariant(); channelNo++)
    {
        /* Recording was going on for this channel then it must be stopped because of record switch event */
        if ((FALSE == GET_CAMERA_MASK_BIT(cameraMask, channelNo)) || (recordSession[channelNo].recordStatus != RECORD_ON))
        {
            continue;
        }

        if (TRUE == recordWriterStatus[channelNo])
        {
            STATE_OVERRIDE_PRINT(channelNo, recordStateStr[recordSession[channelNo].recordStatus], recordStateStr[RECORD_DRIVE_SWITCH]);
        }

        recordSession[channelNo].recordStatus = RECORD_DRIVE_SWITCH;
        recordWriterStatus[channelNo] = TRUE;
        sendSingalF = TRUE;
        DPRINT(RECORD_MANAGER, "record session state: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_DRIVE_SWITCH]);
    }

    if (sendSingalF == TRUE)
    {
        pthread_cond_signal(&frameWrCondSignal);
    }

    MUTEX_UNLOCK(frameWrCondMutex);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was used to register the notify function on hard disk event (like formatting
 *          disk). It will stop all running recording because of user has request to format the disk.
 * @param   cameraMask
 * @return  SUCCESS/FAIL
 */
BOOL StopCameraRecordOnStorageFull(CAMERA_BIT_MASK_t cameraMask)
{
    UINT8	channelNo;
    UINT8	record;
    UINT8	recordType;

    MUTEX_LOCK(frameWrCondMutex);
    for(channelNo = 0; channelNo < getMaxCameraForCurrentVariant(); channelNo++)
    {
        if ((FALSE == GET_CAMERA_MASK_BIT(cameraMask, channelNo)) || (recordSession[channelNo].recordStatus == RECORD_OFF))
        {
            continue;
        }

        MUTEX_LOCK(recordSession[channelNo].dataMutex);
        recordType = recordSession[channelNo].recordType;
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

        /* Detect how many recording was started for this channel */
        for(record = 0; record < MAX_RECORD; record++)
        {
            if (GET_BIT(recordType, record) == 0)
            {
                continue;
            }

            stopInternalRecord(channelNo, record, TRUE);
            MUTEX_LOCK(recordSession[channelNo].dataMutex);
            if(recordSession[channelNo].recordFailReason == EVNT_REC_STOP_DISK_FULL)
            {
                MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
                continue;
            }

            recordSession[channelNo].recordFailReason = EVNT_REC_STOP_DISK_FULL;
            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
            writeRecordingEvent(channelNo, recordType, REC_FAIL, EVNT_REC_STOP_DISK_FULL);
            CameraEventNotify(channelNo, RECORDING_FAIL, ACTIVE);
        }
    }
    MUTEX_UNLOCK(frameWrCondMutex);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to stop all running recording because of hdd group is changed
 * @return  SUCCESS/FAIL
 */
void StopCameraRecordOnHddGroupChange(void)
{
    UINT8	channelNo;
    UINT8	record;
    UINT8	recordType;

    MUTEX_LOCK(frameWrCondMutex);
    for(channelNo = 0; channelNo < getMaxCameraForCurrentVariant(); channelNo++)
    {
        if (recordSession[channelNo].recordStatus == RECORD_OFF)
        {
            continue;
        }

        MUTEX_LOCK(recordSession[channelNo].dataMutex);
        recordType = recordSession[channelNo].recordType;
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

        /* Detect how many recording was started for this channel */
        for(record = 0; record < MAX_RECORD; record++)
        {
            if (GET_BIT(recordType, record) == 0)
            {
                continue;
            }

            stopInternalRecord(channelNo, record, TRUE);
            MUTEX_LOCK(recordSession[channelNo].dataMutex);
            if(recordSession[channelNo].recordFailReason == EVNT_REC_STOP_HDD_GROUP_CHANGED)
            {
                MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
                continue;
            }

            recordSession[channelNo].recordFailReason = EVNT_REC_STOP_HDD_GROUP_CHANGED;
            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
            writeRecordingEvent(channelNo, recordType, REC_FAIL, EVNT_REC_STOP_HDD_GROUP_CHANGED);
            CameraEventNotify(channelNo, RECORDING_FAIL, ACTIVE);
        }
    }
    MUTEX_UNLOCK(frameWrCondMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was used to register the notify function on hard disk event (like disk disk).
 *          It will stop all running recording because of disk fault.
 * @param   recordDisk
 * @return  SUCCESS/FAIL
 */
BOOL StopRecordDueToDiskFaultEvent(RECORD_ON_DISK_e recordDisk)
{
    UINT8			channelNo;
    UINT8			record;
    UINT8			recordType;
    LOG_REC_STOP_e	recStopRes = 0;

    MUTEX_LOCK(frameWrCondMutex);
    for(channelNo = 0; channelNo < getMaxCameraForCurrentVariant(); channelNo++)
    {
        if(recordSession[channelNo].recordStatus == RECORD_OFF)
        {
            continue;
        }

        MUTEX_LOCK(recordSession[channelNo].dataMutex);
        recordType = recordSession[channelNo].recordType;
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

        // detect how many recording was started for this channel
        for(record = 0; record < MAX_RECORD; record++)
        {
            if (GET_BIT(recordType, record))
            {
                stopInternalRecord(channelNo, record, TRUE);
            }
        }

        switch(recordDisk)
        {
            case LOCAL_HARD_DISK:
                recStopRes = EVNT_REC_STOP_DISK_FAULT;
                break;

            case REMOTE_NAS1:
                recStopRes = EVNT_REC_STOP_NDD1_DISCONNECT;
                break;

            case REMOTE_NAS2:
                recStopRes = EVNT_REC_STOP_NDD2_DISCONNECT;
                break;

            default:
                recStopRes = EVNT_REC_STOP_DISK_FAULT;
                break;
        }

        MUTEX_LOCK(recordSession[channelNo].dataMutex);
        if(recordSession[channelNo].recordFailReason == recStopRes)
        {
            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
            continue;
        }

        recordSession[channelNo].recordFailReason = recStopRes;
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
        writeRecordingEvent(channelNo, recordType, REC_FAIL, recStopRes);
        CameraEventNotify(channelNo, RECORDING_FAIL, ACTIVE);
    }
    MUTEX_UNLOCK(frameWrCondMutex);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was callback of entiring pre alarm record stream.
 * @param   respParam
 */
static void entPreAlrmCallback(const CI_STREAM_RESP_PARAM_t *respParam)
{
    UINT8 camIndex = GET_STREAM_INDEX(respParam->camIndex);

    if (camIndex >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    switch (respParam->respCode)
    {
        case CI_STREAM_RESP_START:
        {
            BOOL preAlarmRecordF = FALSE;
            if((respParam->cmdStatus == CMD_SUCCESS) || (respParam->cmdStatus == CMD_STREAM_ALREADY_ON))
            {
                preAlarmRecordF = TRUE;
            }

            MUTEX_LOCK(recordSession[camIndex].dataMutex);
            recordSession[camIndex].preAlrmRcrdStrm = preAlarmRecordF;
            MUTEX_UNLOCK(recordSession[camIndex].dataMutex);
            DPRINT(RECORD_MANAGER, "pre-alarm record stream start: [camera=%d], [status=%d]", camIndex, preAlarmRecordF);
        }
        break;

        case CI_STREAM_RESP_CLOSE:
        {
            MUTEX_LOCK(recordSession[camIndex].dataMutex);
            recordSession[camIndex].preAlrmRcrdStrm = FALSE;
            MUTEX_UNLOCK(recordSession[camIndex].dataMutex);
            DPRINT(RECORD_MANAGER, "pre-alarm record stream stop: [camera=%d]", camIndex);
        }
        break;

        default:
        {
            /* Noting to do */
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was callback of entiring pre alarm record stream.
 * @param   respParam
 */
static void entPreCosecCallback(const CI_STREAM_RESP_PARAM_t *respParam)
{
    UINT8 camIndex = GET_STREAM_INDEX(respParam->camIndex);

    if (camIndex >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    switch (respParam->respCode)
    {
        case CI_STREAM_RESP_START:
        {
            BOOL preCosecRecordF = FALSE;
            if ((respParam->cmdStatus == CMD_SUCCESS) || (respParam->cmdStatus == CMD_STREAM_ALREADY_ON))
            {
                preCosecRecordF = TRUE;
            }

            MUTEX_LOCK(recordSession[camIndex].dataMutex);
            recordSession[camIndex].preCosecRcrdStrm = preCosecRecordF;
            MUTEX_UNLOCK(recordSession[camIndex].dataMutex);
            DPRINT(RECORD_MANAGER, "pre-cosec record stream start: [camera=%d], [status=%d]", camIndex, preCosecRecordF);
        }
        break;

        case CI_STREAM_RESP_CLOSE:
        {
            MUTEX_LOCK(recordSession[camIndex].dataMutex);
            recordSession[camIndex].preCosecRcrdStrm = FALSE;
            MUTEX_UNLOCK(recordSession[camIndex].dataMutex);
            DPRINT(RECORD_MANAGER, "pre-cosec record stream stop: [camera=%d]", camIndex);
        }
        break;

        default:
        {
            /* Noting to do */
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was callback of start recording stream.
 * @param   respParam
 */
static void startStreamCallback(const CI_STREAM_RESP_PARAM_t *respParam)
{
    UINT8   channelNo = GET_STREAM_INDEX(respParam->camIndex);
    UINT8   recordType = 0;
    UINT8   record;

    if (channelNo >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    if (CI_STREAM_RESP_MEDIA != respParam->respCode)
    {
        DPRINT(RECORD_MANAGER, "stream callback status: [camera=%d], [cmdStatus=%d], [state=%s]",
               channelNo, respParam->cmdStatus, recordStateStr[recordSession[channelNo].recordStatus]);
    }

    switch (respParam->respCode)
    {
        case CI_STREAM_RESP_START:
        {
            MUTEX_LOCK(frameWrCondMutex);
            if(recordSession[channelNo].recordStatus == RECORD_ON_WAIT)
            {
                if(respParam->cmdStatus == CMD_SUCCESS)
                {
                    if (TRUE == recordWriterStatus[channelNo])
                    {
                        STATE_OVERRIDE_PRINT(channelNo, recordStateStr[recordSession[channelNo].recordStatus], recordStateStr[RECORD_ON]);
                        recordOnWaitStatus[channelNo] = TRUE;
                    }
                    else
                    {
                        recordSession[channelNo].recordStatus = RECORD_ON;
                        recordSession[channelNo].recordFailReason = MAX_EVNT_REC_STOP;
                        DPRINT(RECORD_MANAGER, "record session state: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_ON]);

                        MUTEX_LOCK(recordSession[channelNo].dataMutex);
                        recordType = recordSession[channelNo].recordType;
                        recordSession[channelNo].recordFailReason = MAX_EVNT_REC_STOP;
                        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

                        writeRecordingEvent(channelNo, recordType, REC_START, MAX_EVNT_REC_STOP);
                        CameraEventNotify(channelNo, RECORDING_FAIL, INACTIVE);
                    }

                    recordWriterStatus[channelNo] = TRUE;
                    pthread_cond_signal(&frameWrCondSignal);
                    MUTEX_UNLOCK(frameWrCondMutex);
                }
                else
                {
                    if (TRUE == recordWriterStatus[channelNo])
                    {
                        STATE_OVERRIDE_PRINT(channelNo, recordStateStr[recordSession[channelNo].recordStatus], recordStateStr[RECORD_OFF_WAIT]);
                    }

                    if(recordSession[channelNo].recordStatus != RECORD_OFF)
                    {
                        recordSession[channelNo].recordStatus = RECORD_OFF_WAIT;
                        DPRINT(RECORD_MANAGER, "record session state: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_OFF_WAIT]);
                    }

                    MUTEX_LOCK(recordSession[channelNo].dataMutex);
                    recordType = recordSession[channelNo].recordType;
                    if(recordSession[channelNo].recordFailReason != EVNT_REC_STOP_VIDEO_LOSS)
                    {
                        recordSession[channelNo].recordFailReason = EVNT_REC_STOP_VIDEO_LOSS;
                        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

                        writeRecordingEvent(channelNo, recordType, REC_FAIL, EVNT_REC_STOP_VIDEO_LOSS);
                        CameraEventNotify(channelNo, RECORDING_FAIL, ACTIVE);
                    }
                    else
                    {
                        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
                    }

                    recordWriterStatus[channelNo] = TRUE;
                    pthread_cond_signal(&frameWrCondSignal);
                    MUTEX_UNLOCK(frameWrCondMutex);

                    //NOTE: Cosec timer not operate in lock
                    if (GET_BIT(recordType, COSEC_RECORD))
                    {
                        DeleteTimer(&postCosecRecTimerHandle[channelNo]);
                    }
                }
            }
            else if(recordSession[channelNo].recordStatus == RECORD_RESTART_CLEANUP)
            {
                if(respParam->cmdStatus == CMD_SUCCESS)
                {
                    if (TRUE == recordWriterStatus[channelNo])
                    {
                        recordRestartWaitStatus[channelNo] = TRUE;
                    }
                    else
                    {
                        recordSession[channelNo].recordStatus = RECORD_RESTART_INIT;
                        recordWriterStatus[channelNo] = TRUE;
                        pthread_cond_signal(&frameWrCondSignal);
                        DPRINT(RECORD_MANAGER, "record session state changed: [camera=%d], [state=%s --> %s]",
                               channelNo, recordStateStr[RECORD_RESTART_CLEANUP], recordStateStr[RECORD_RESTART_INIT]);
                    }
                }
                else
                {
                    if (TRUE == recordWriterStatus[channelNo])
                    {
                        STATE_OVERRIDE_PRINT(channelNo, recordStateStr[recordSession[channelNo].recordStatus], recordStateStr[RECORD_OFF_WAIT]);
                    }

                    if(recordSession[channelNo].recordStatus != RECORD_OFF)
                    {
                        recordSession[channelNo].recordStatus = RECORD_OFF_WAIT;
                        recordWriterStatus[channelNo] = TRUE;
                        pthread_cond_signal(&frameWrCondSignal);
                        DPRINT(RECORD_MANAGER, "record session state: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_OFF_WAIT]);
                    }

                    MUTEX_LOCK(recordSession[channelNo].dataMutex);
                    recordType = recordSession[channelNo].recordType;
                    if(recordSession[channelNo].recordFailReason != EVNT_REC_STOP_VIDEO_LOSS)
                    {
                        recordSession[channelNo].recordFailReason = EVNT_REC_STOP_VIDEO_LOSS;
                        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

                        writeRecordingEvent(channelNo, recordType, REC_FAIL, EVNT_REC_STOP_VIDEO_LOSS);
                        CameraEventNotify(channelNo, RECORDING_FAIL, ACTIVE);
                    }
                    else
                    {
                        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
                    }
                }

                MUTEX_UNLOCK(frameWrCondMutex);
            }
            else if(recordSession[channelNo].recordStatus == RECORD_ON)
            {
                recordWriterStatus[channelNo] = TRUE;
                pthread_cond_signal(&frameWrCondSignal);
                MUTEX_UNLOCK(frameWrCondMutex);
            }
            else
            {
                MUTEX_UNLOCK(frameWrCondMutex);
                EPRINT(RECORD_MANAGER, "unhandled record session state: [camera=%d], [state=%s]",
                       channelNo, recordStateStr[recordSession[channelNo].recordStatus]);
            }
        }
        break;

        case CI_STREAM_RESP_MEDIA:
        {
            MUTEX_LOCK(frameWrCondMutex);
            if(recordSession[channelNo].recordStatus == RECORD_ON)
            {
                recordWriterStatus[channelNo] = TRUE;
                pthread_cond_signal(&frameWrCondSignal);
            }
            MUTEX_UNLOCK(frameWrCondMutex);
        }
        break;

        case CI_STREAM_RESP_RETRY:
        {
            EPRINT(RECORD_MANAGER, "camera stream in retry state: [camera=%d]", channelNo);
            MUTEX_LOCK(frameWrCondMutex);
            if((recordSession[channelNo].recordStatus == RECORD_ON) || (recordSession[channelNo].recordStatus == RECORD_DRIVE_SWITCH))
            {
                if (TRUE == recordWriterStatus[channelNo])
                {
                    STATE_OVERRIDE_PRINT(channelNo, recordStateStr[recordSession[channelNo].recordStatus], recordStateStr[RECORD_RESTART_CLEANUP]);
                }

                recordSession[channelNo].recordStatus = RECORD_RESTART_CLEANUP;
                recordSession[channelNo].recordFailReason = EVNT_REC_STOP_VIDEO_LOSS;
                recordWriterStatus[channelNo] = TRUE;
                pthread_cond_signal(&frameWrCondSignal);
                DPRINT(RECORD_MANAGER, "record session state: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_RESTART_CLEANUP]);
            }
            else
            {
                EPRINT(RECORD_MANAGER, "unhandled state in camera stream retry state: [camera=%d], [state=%s]",
                       channelNo, recordStateStr[recordSession[channelNo].recordStatus]);
            }
            MUTEX_UNLOCK(frameWrCondMutex);
        }
        break;

        case CI_STREAM_RESP_CLOSE:
        {
            EPRINT(RECORD_MANAGER, "camera stream in close state: [camera=%d], [state=%s]",
                   channelNo, recordStateStr[recordSession[channelNo].recordStatus]);
            MUTEX_LOCK(frameWrCondMutex);
            if(recordSession[channelNo].recordStatus != RECORD_OFF)
            {
                MUTEX_LOCK(recordSession[channelNo].dataMutex);
                recordType = recordSession[channelNo].recordType;
                MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

                // detect how many recording was started for this channel
                for(record = 0; record < MAX_RECORD; record++)
                {
                    if (GET_BIT(recordType, record))
                    {
                        stopInternalRecord(channelNo, record, TRUE);
                    }
                }
            }
            else
            {
                EPRINT(RECORD_MANAGER, "unhandled state in camera stream close state: [camera=%d], [state=%s]",
                       channelNo, recordStateStr[recordSession[channelNo].recordStatus]);
            }

            MUTEX_UNLOCK(frameWrCondMutex);
            ExitPreAlrmRecStrm(channelNo);
        }
        break;

        default:
        {
            /* Noting to do */
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function starts new record session for given channel in input. if recording of this
 *          channel was already running then it will not create a new session for recording.
 * @param   channelNo
 * @param   recType
 * @param   recDuration
 * @param   advncDetail
 * @return
 */
NET_CMD_STATUS_e StartRecord(UINT8 channelNo, RECORD_TYPE_e recType, UINT32 recDuration, CHARPTR advncDetail)
{
    BOOL						status = CMD_SUCCESS;
    UINT32						timerCnt;
    UINT32						remainingTime;
    STORAGE_HEALTH_STATUS_e		diskHelStatus;
    HDD_CONFIG_t                hddConfig;
    MANUAL_RECORD_CONFIG_t		manualRecordCfg;

    // Validate channel number
    if (channelNo >= getMaxCameraForCurrentVariant())
    {
        return CMD_MAX_CAMERA_CONFIGURED;
    }

    DPRINT(RECORD_MANAGER, "start record: [camera=%d], [type=%s]", channelNo, recTypeStr[recType]);
    if(recType == SCHEDULE_RECORD)
    {
        return CMD_PROCESS_ERROR;
    }

    MUTEX_LOCK(frameWrCondMutex);
    if(recType == ALARM_RECORD)
    {
        MUTEX_LOCK(recordSession[channelNo].dataMutex);
        recordSession[channelNo].alrmRecordCnt++;
        if (GET_BIT(recordSession[channelNo].recordType, recType))
        {
            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
            MUTEX_UNLOCK(frameWrCondMutex);
            DeleteTimer(&postAlarmRecTimerHandle[channelNo]);
            DPRINT(RECORD_MANAGER, "record already running: [camera=%d], [type=%s]", channelNo, recTypeStr[recType]);
            return CMD_SUCCESS; // Dont start record again so return success from here.
        }
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
    }
    else if(recType == MANUAL_RECORD)
    {
        ReadHddConfig(&hddConfig);
        diskHelStatus = GetCameraVolumeHealthStatus(channelNo, hddConfig.recordDisk);
        if (diskHelStatus == STRG_HLT_NO_DISK)
        {
            status = CMD_NO_DISK_FOUND;
        }
        else if (diskHelStatus == STRG_HLT_DISK_FULL)
        {
            status = CMD_REC_MEDIA_FULL;
        }
        else if (diskHelStatus != STRG_HLT_DISK_NORMAL)
        {
            status = GiveMediaStatus(hddConfig.recordDisk);
            EPRINT(RECORD_MANAGER, "disk status not proper: [camera=%d], [status=%d]", channelNo, status);
        }
        else
        {
            ReadSingleManualRecordConfig(channelNo, &manualRecordCfg);
            if(manualRecordCfg.manualRecordStatus == ENABLE)
            {
                MUTEX_LOCK(recordSession[channelNo].dataMutex);
                if (GET_BIT(recordSession[channelNo].recordType, MANUAL_RECORD))
                {
                    status = CMD_MANUAL_RECORDING_ON;
                }
                else
                {
                    snprintf(recordSession[channelNo].manualRecUserName, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", advncDetail);
                }
                MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
            }
            else
            {
                status = CMD_MAN_RECORD_DISABLED;
            }
            DPRINT(RECORD_MANAGER, "manual record status: [camera=%d], [status=%d]", channelNo, status);
        }
    }
    else if(recType == COSEC_RECORD)
    {
        MUTEX_UNLOCK(frameWrCondMutex);
        //NOTE: Cosec timer not operate in lock
        if(postCosecRecTimerHandle[channelNo] != INVALID_TIMER_HANDLE)
        {
            if(GetRemainingTime(postCosecRecTimerHandle[channelNo], &remainingTime) == SUCCESS)
            {
                timerCnt = CONVERT_SEC_TO_TIMER_COUNT(recDuration * SEC_IN_ONE_MIN);
                if(timerCnt > remainingTime)
                {
                    if(ReloadTimer(postCosecRecTimerHandle[channelNo], timerCnt) == SUCCESS)
                    {
                        status = CMD_PROCESS_ERROR;
                        DPRINT(RECORD_MANAGER, "cosec recording reloaded: [camera=%d], [timerCnt=%d]", channelNo, timerCnt);
                    }
                }
                else
                {
                    status = CMD_PROCESS_ERROR;
                    DPRINT(RECORD_MANAGER, "cosec recording already running: [camera=%d], [reloadCnt=%d], [remainingCnt=%d]", channelNo, timerCnt, remainingTime);
                }
            }
        }

        if(status == CMD_SUCCESS)
        {
            snprintf(recordSession[channelNo].cosecRecUserName, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", advncDetail);
        }

        MUTEX_LOCK(frameWrCondMutex);
    }

    if(status == CMD_SUCCESS)
    {
        status = startInternalRecord(channelNo, recType, recDuration);
        if(status != CMD_SUCCESS)
        {
            EPRINT(RECORD_MANAGER, "recording not started: [camera=%d], [type=%s]", channelNo, recTypeStr[recType]);
        }
    }

    MUTEX_UNLOCK(frameWrCondMutex);
    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   writeRecordingEvent
 * @param   channelNo
 * @param   recType
 * @param   action
 * @param   eventDetailType
 */
static void writeRecordingEvent(UINT8 channelNo, UINT8 recType, RCD_STATE_e action, LOG_REC_STOP_e eventDetailType)
{
    UINT8					record;
    LOG_EVENT_TYPE_e		eventType = LOG_CAMERA_EVENT;
    LOG_EVENT_SUBTYPE_e		eventLogData;
    CHAR					eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR					advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(channelNo));
    for(record = 0; record < MAX_RECORD; record++)
    {
        if (GET_BIT(recType, record) == 0)
        {
            continue;
        }

        switch(record)
        {
            case MANUAL_RECORD:
            {
                eventLogData = LOG_MANUAL_RECORDING;
                if(eventDetailType == MAX_EVNT_REC_STOP)
                {
                    snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", recordSession[channelNo].manualRecUserName);
                }
                else
                {
                    snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", recordStopStateStr[eventDetailType]);
                }
            }
            break;

            case ALARM_RECORD:
            {
                eventLogData = LOG_ALARM_RECORDING;
                if(eventDetailType == MAX_EVNT_REC_STOP)
                {
                    advncDetail[0] = '\0';
                }
                else
                {
                    snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", recordStopStateStr[eventDetailType]);
                }
            }
            break;

            case SCHEDULE_RECORD:
            {
                eventLogData = LOG_SCHEDULE_RECORDING;
                if(eventDetailType == MAX_EVNT_REC_STOP)
                {
                    advncDetail[0] = '\0';
                }
                else
                {
                    snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", recordStopStateStr[eventDetailType]);
                }
            }
            break;

            case COSEC_RECORD:
            {
                eventType = LOG_COSEC_EVENT;
                eventLogData = LOG_COSEC_RECORDING;
                if(eventDetailType == MAX_EVNT_REC_STOP)
                {
                    snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", recordSession[channelNo].cosecRecUserName);
                }
                else
                {
                    snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", recordStopStateStr[eventDetailType]);
                }
            }
            break;

            default:
            {
                /* Nothing to do */
            }
            return;
        }

        WriteEvent(eventType, eventLogData, eventDetail, advncDetail, (LOG_EVENT_STATE_e)action);
        recordSession[channelNo].recordHeathState[record] = action;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function starts new record session for given channel in input. If recording of this
 *          channel was already running then it will not create a new session for recording.
 * @param   channelNo
 * @param   recType
 * @param   recDuration
 * @return
 */
static NET_CMD_STATUS_e startInternalRecord(UINT8 channelNo, RECORD_TYPE_e recType, UINT32 recDuration)
{
    RECORD_STATE_e  recordStatus;
    TIMER_INFO_t    timerInfo;

    if(recType == SCHEDULE_RECORD)
    {
        MUTEX_LOCK(frameWrCondMutex);
        MUTEX_LOCK(recordSession[channelNo].dataMutex);
        if (GET_BIT(recordSession[channelNo].recordType, SCHEDULE_RECORD))
        {
            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
            MUTEX_UNLOCK(frameWrCondMutex);
            return CMD_SUCCESS;
        }
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
    }
    else if(recType == COSEC_RECORD)
    {
        recDuration = (recDuration * SEC_IN_ONE_MIN);
        timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(recDuration);
        timerInfo.data = channelNo;
        timerInfo.funcPtr = stopCosecRec;

        //NOTE : Cosec timer not operate in lock
        StartTimer(timerInfo, &postCosecRecTimerHandle[channelNo]);
        DPRINT(RECORD_MANAGER, "stop cosec recording time: [camera=%d], [time=%dsec]", channelNo, recDuration);
        MUTEX_LOCK(frameWrCondMutex);
    }
    else
    {
        MUTEX_LOCK(frameWrCondMutex);
    }

    if(recordSession[channelNo].recordStatus == RECORD_OFF)
    {
        MUTEX_LOCK(recordSession[channelNo].dataMutex);
        recordSession[channelNo].recordType = (1 << recType);
        recordSession[channelNo].preRecdStrmSkip = NO;
        GetLocalTime(&recordSession[channelNo].localTime);
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

        if (TRUE == recordWriterStatus[channelNo])
        {
            STATE_OVERRIDE_PRINT(channelNo, recordStateStr[recordSession[channelNo].recordStatus], recordStateStr[RECORD_ON_WAIT]);
        }

        recordSession[channelNo].recordStatus = RECORD_ON_WAIT;
        recordWriterStatus[channelNo] = TRUE;
        pthread_cond_signal(&frameWrCondSignal);
    }
    else
    {
        recordStatus = recordSession[channelNo].recordStatus;
        MUTEX_LOCK(recordSession[channelNo].dataMutex);
        recordSession[channelNo].recordType |= (1 << recType);
        if(recordStatus == RECORD_ON)
        {
            recordSession[channelNo].recordFailReason = MAX_EVNT_REC_STOP;
            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
            writeRecordingEvent(channelNo, (1 << recType), REC_START, MAX_EVNT_REC_STOP);
            EventDetectFunc(channelNo, RECORDING_START, ACTIVE);
            CameraEventNotify(channelNo, RECORDING_FAIL, INACTIVE);
        }
        else
        {
            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
        }
    }

    MUTEX_UNLOCK(frameWrCondMutex);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function starts stops already running recording session. It was closed only MANUAL_RECORD
 *          and ALARM_RECORD type of recording. If SCHEDULE_RECORD recording for same channel was running
 *          then it will not closed the recording session.
 * @param   channelNo
 * @param   recordType
 * @param   advncDetail
 * @return
 */
NET_CMD_STATUS_e StopRecord(UINT8 channelNo, RECORD_TYPE_e recordType, CHARPTR advncDetail)
{
    TIMER_INFO_t				timerInfo;
    MANUAL_RECORD_CONFIG_t		manualRecordCfg;
    ALARM_RECORD_CONFIG_t		alarmRecordCfg;
    RECORD_STATE_e				recordStatus;

    // Validate channel number
    if(channelNo >= getMaxCameraForCurrentVariant())
    {
        EPRINT(RECORD_MANAGER, "invld camera id: [camera=%d]", channelNo);
        return CMD_INVALID_FIELD_ID;
    }

    DPRINT(RECORD_MANAGER, "stop record: [camera=%d], [type=%s]", channelNo, recTypeStr[recordType]);
    if(recordType == ALARM_RECORD)
    {
        MUTEX_LOCK(frameWrCondMutex);
        MUTEX_LOCK(recordSession[channelNo].dataMutex);
        if(recordSession[channelNo].alrmRecordCnt == 0)
        {
            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
            recordSession[channelNo].recordHeathState[recordType] = REC_STOP;
            MUTEX_UNLOCK(frameWrCondMutex);
            return CMD_SUCCESS;
        }

        recordSession[channelNo].alrmRecordCnt--;
        if(recordSession[channelNo].alrmRecordCnt)
        {
            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
            MUTEX_UNLOCK(frameWrCondMutex);
            return CMD_SUCCESS;
        }
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
        MUTEX_UNLOCK(frameWrCondMutex);

        ReadSingleAlarmRecordConfig(channelNo, &alarmRecordCfg);
        if(alarmRecordCfg.postRecordTime > 0)
        {
            if(postAlarmRecTimerHandle[channelNo] == INVALID_TIMER_HANDLE)
            {
                // if record type was alarm record type and pre alarm record time configured then start timer and stop record from timer callback
                timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(alarmRecordCfg.postRecordTime);
                timerInfo.data = channelNo;
                timerInfo.funcPtr = postAlarmRecCallback;
                StartTimer(timerInfo, &postAlarmRecTimerHandle[channelNo]);
            }
            return CMD_SUCCESS;
        }

        MUTEX_LOCK(frameWrCondMutex);
    }
    else if(recordType == MANUAL_RECORD)
    {
        ReadSingleManualRecordConfig(channelNo, &manualRecordCfg);
        if(manualRecordCfg.manualRecordStatus == DISABLE)
        {
            return CMD_MAN_RECORD_DISABLED;
        }

        MUTEX_LOCK(frameWrCondMutex);
        recordStatus = recordSession[channelNo].recordStatus;
        MUTEX_LOCK(recordSession[channelNo].dataMutex);
        if((recordStatus == RECORD_OFF) || (GET_BIT(recordSession[channelNo].recordType, recordType) == 0))
        {
            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
            MUTEX_UNLOCK(frameWrCondMutex);
            return CMD_MANUAL_RECORDING_OFF;
        }

        snprintf(recordSession[channelNo].manualRecUserName, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", advncDetail);
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
    }
    else
    {
        return CMD_PROCESS_ERROR;
    }

    stopInternalRecord(channelNo, recordType, FALSE);
    MUTEX_UNLOCK(frameWrCondMutex);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is timer callback which serves post alarm record duration and stops alarm recording
 *          of given channel.
 * @param   data
 */
static void postAlarmRecCallback(UINT32 data)
{
    if (data >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    UINT8 camIndex = (UINT8)data;
    DeleteTimer(&postAlarmRecTimerHandle[camIndex]);
    if(stopInternalRecord(camIndex, ALARM_RECORD, FALSE) == FAIL)
    {
        EPRINT(RECORD_MANAGER, "fail to stop alarm recording: [camera=%d]", camIndex);
    }
    else
    {
        DPRINT(RECORD_MANAGER, "alarm recording stopped: [camera=%d]", camIndex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is timer callback which stops COSEC recording
 * @param   data
 * @return
 */
static void stopCosecRec(UINT32 data)
{
    if (data >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    UINT8 camIndex = (UINT8)data;
    DeleteTimer(&postCosecRecTimerHandle[camIndex]);
    if(stopInternalRecord(camIndex, COSEC_RECORD, FALSE) == FAIL)
    {
        EPRINT(RECORD_MANAGER, "fail to stop cosec recording: [camera=%d]", camIndex);
    }
    else
    {
        DPRINT(RECORD_MANAGER, "cosec recording stopped: [camera=%d]", camIndex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function starts stops already running recording session. It was closed only SCHEDULE_RECORD.
 * @param   channelNo
 * @param   recordType
 * @param   forceStop
 * @return
 */
static BOOL stopInternalRecord(UINT8 channelNo, RECORD_TYPE_e recordType, BOOL forceStop)
{
    UINT8			count, curRecCnt = 0;
    RECORD_STATE_e	recordStatus;
    CAMERA_CONFIG_t	camConfig;
    struct tm 		brokenTime = { 0 };
    RCD_STATE_e		recordState = REC_STOP;

    GetLocalTimeInBrokenTm(&brokenTime);
    if(SCHEDULE_RECORD == recordType)
    {
        if(valSchdlRecordTime(channelNo, &brokenTime) == SUCCESS)
        {
            recordState = REC_FAIL;
        }
    }

    /* PARASOFT : Rule BD-TRS-DLOCK marked false positive - Recursive mutex */
    MUTEX_LOCK(frameWrCondMutex);
    recordSession[channelNo].recordHeathState[recordType] = recordState;
    MUTEX_LOCK(recordSession[channelNo].dataMutex);

    if(ALARM_RECORD == recordType)
    {
        if(forceStop == FALSE)
        {
            ReadSingleCameraConfig(channelNo, &camConfig);
            if((camConfig.camera == ENABLE) && (recordSession[channelNo].alrmRecordCnt != 0))
            {
                MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
                MUTEX_UNLOCK(frameWrCondMutex);
                EPRINT(RECORD_MANAGER, "alarm record count not zero: [camera=%d]", channelNo);
                return FAIL;
            }
        }
    }
    else
    {
        if (GET_BIT(recordSession[channelNo].recordType, recordType) == 0)
        {
            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
            MUTEX_UNLOCK(frameWrCondMutex);
            return SUCCESS;
        }
    }

    // detect how many recording was started for this channel
    for(count = 0; count < MAX_RECORD; count++)
    {
        if(GET_BIT(recordSession[channelNo].recordType, count))
        {
            curRecCnt++;
        }
    }

    MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

    // check recoding of given channel was started previously
    if((recordSession[channelNo].recordStatus != RECORD_OFF) && (curRecCnt >= 2))
    {
        recordStatus = recordSession[channelNo].recordStatus;
        MUTEX_LOCK(recordSession[channelNo].dataMutex);
        // stop recording due to recordType in current recording
        recordSession[channelNo].recordType &= ~(1 << recordType);
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

        if((recordStatus != RECORD_RESTART_CLEANUP) && (recordStatus != RECORD_ON_WAIT))
        {
            writeRecordingEvent(channelNo, (1 << recordType), REC_STOP, MAX_EVNT_REC_STOP);
        }

        DPRINT(RECORD_MANAGER, "recording stopped: [camera=%d], [type=%s]", channelNo, recTypeStr[recordType]);
    }
    // recording was running and have only recordType recording
    else if(recordSession[channelNo].recordStatus != RECORD_OFF)
    {
        recordStatus = recordSession[channelNo].recordStatus;
        MUTEX_LOCK(recordSession[channelNo].dataMutex);
        // set recording type was none.
        recordSession[channelNo].recordType &= (~(1 << recordType));
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

        if((recordStatus != RECORD_RESTART_CLEANUP) && (recordStatus != RECORD_ON_WAIT))
        {
            writeRecordingEvent(channelNo, (1 << recordType), REC_STOP, MAX_EVNT_REC_STOP);
        }

        if ((recordStatus == RECORD_ON) && (TRUE == recordWriterStatus[channelNo]))
        {
            STATE_OVERRIDE_PRINT(channelNo, recordStateStr[recordSession[channelNo].recordStatus], recordStateStr[RECORD_OFF_WAIT]);
            recordOffWaitStatus[channelNo] = TRUE;
            pthread_cond_signal(&frameWrCondSignal);
        }
        else
        {
            recordSession[channelNo].recordStatus = RECORD_OFF_WAIT;
            DPRINT(RECORD_MANAGER, "record session state: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_OFF_WAIT]);
            recordWriterStatus[channelNo] = TRUE;
            pthread_cond_signal(&frameWrCondSignal);
        }

        DPRINT(RECORD_MANAGER, "all recording stopped: [camera=%d]", channelNo);
    }

    MUTEX_UNLOCK(frameWrCondMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is threading function for individual camera. It will received frame from camera
 *          interface and passes to disk manager for further process (Write into hard disk).
 * @param   threadArg
 * @return
 */
static VOIDPTR writeFrame(VOIDPTR threadArg)
{
    BOOL                        writeFrame;
    UINT8                       channelNo;
    UINT8                       recordChannelMax;
    UINT8                       recordType;
    UINT32                      prevFrameTime = 0;
    INT32                       timeDiff;
    UINT32                      preRecordTime = 0;
    UINT8PTR                    streadmDataPtr;
    UINT32                      streamLen, pendFrame = 0;
    ALARM_RECORD_CONFIG_t       alarmRecordCfg;
    STREAM_STATUS_INFO_t        *streamInfo;
    METADATA_INFO_t             metaDataInfo;
    COSEC_REC_PARAM_CONFIG_t	cosecRecParam;
    HDD_CONFIG_t                storageConfig;
    LOG_REC_STOP_e              recStopRes;
    UINT32                      tmpCamIdx = 0;
    BOOL                        stopAllRecording = FALSE;
    RECORD_TYPE_e               recType = 0;
    BOOL                        frameSkipF = FALSE;
    BOOL                        adaptiveRecordingF = FALSE;
    STORAGE_HEALTH_STATUS_e     diskStatus = STRG_HLT_MAX;
    CI_BUFFER_READ_POS_e        readPos;
    RECORD_TYPE_e               record;
    UINT32                      errorCode = INVALID_ERROR_CODE;

    THREAD_START("REC_MANAGER");

    setpriority(PRIO_PROCESS, PRIO_PROCESS, -1);
    recordChannelMax = getMaxCameraForCurrentVariant();

    while(TRUE)
    {
        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        MUTEX_LOCK(frameWrCondMutex);
        if(recTerminateFlg == TRUE)
        {
            MUTEX_UNLOCK(frameWrCondMutex);
            break;
        }

        /* Is recording on for any cameras? */
        for (channelNo = 0; channelNo < recordChannelMax; channelNo++)
        {
            if (TRUE == recordWriterStatus[channelNo])
            {
                break;
            }
        }

        /* Recording is not running for any camera */
        if (channelNo >= recordChannelMax)
        {
            /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
            pthread_cond_wait(&frameWrCondSignal, &frameWrCondMutex);
        }
        MUTEX_UNLOCK(frameWrCondMutex);

        for (channelNo = 0; channelNo < recordChannelMax; channelNo++)
        {
            /* Get frame from camera interface. camera interface was buffering the media dat. here we are making loop till
             * end of media frame buffer. Do not Open this lock in between any operation in any state untill the end of that state */
            /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
            MUTEX_LOCK(frameWrCondMutex);
            if (FALSE == recordWriterStatus[channelNo])
            {
                MUTEX_UNLOCK(frameWrCondMutex);
                continue;
            }

            /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
            switch(recordSession[channelNo].recordStatus)
            {
                case RECORD_ON_WAIT:
                {
                    /** Start DiskManager session */
                    if(StartRecordSession(channelNo) == FAIL)
                    {
                        MUTEX_LOCK(recordSession[channelNo].dataMutex);
                        recordType = recordSession[channelNo].recordType;
                        recordSession[channelNo].recordType = 0;
                        recordSession[channelNo].alrmRecordCnt = 0;
                        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

                        recordSession[channelNo].recordStatus = RECORD_OFF;

                        /* Do not give disconnection log when drive is in recovery mode */
                        if(DISK_ACT_RECOVERY != GetStorageDriveStatus(MAX_RECORDING_MODE))
                        {
                            ReadHddConfig(&storageConfig);
                            if ((storageConfig.recordDisk == REMOTE_NAS1) || (storageConfig.recordDisk == REMOTE_NAS2))
                            {
                                recStopRes = (storageConfig.recordDisk - REMOTE_NAS1) + EVNT_REC_STOP_NDD1_DISCONNECT;
                            }
                            else
                            {
                                diskStatus = GetCameraVolumeHealthStatus(channelNo, storageConfig.recordDisk);
                                if ((diskStatus == STRG_HLT_NO_DISK) || (diskStatus == STRG_HLT_MAX))
                                {
                                    recStopRes = MAX_EVNT_REC_STOP;
                                }
                                else if (diskStatus == STRG_HLT_DISK_FULL)
                                {
                                    recStopRes = EVNT_REC_STOP_DISK_FULL;
                                }
                                else
                                {
                                    recStopRes = EVNT_REC_STOP_DISK_FAULT;
                                }
                            }

                            MUTEX_LOCK(recordSession[channelNo].dataMutex);
                            if(recordSession[channelNo].recordFailReason != recStopRes)
                            {
                                recordSession[channelNo].recordFailReason = recStopRes;
                                MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

                                writeRecordingEvent(channelNo, recordType, REC_FAIL, recStopRes);
                                CameraEventNotify(channelNo, RECORDING_FAIL, ACTIVE);
                            }
                            else
                            {
                                MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
                                for(record = 0; record < MAX_RECORD; record++)
                                {
                                    if (GET_BIT(recordType, record))
                                    {
                                        recordSession[channelNo].recordHeathState[record] = REC_FAIL;
                                    }
                                }
                            }
                        }

                        if (GET_BIT(recordType, COSEC_RECORD))
                        {
                            //NOTE : Cosec timer not operate in lock
                            MUTEX_UNLOCK(frameWrCondMutex);
                            DeleteTimer(&postCosecRecTimerHandle[channelNo]);
                            MUTEX_LOCK(frameWrCondMutex);
                        }
                    }
                    else
                    {
                        /** For Main stream 0-63 index is used and for sub stream 64-127 index is used */
                        tmpCamIdx = getRecordChannelNo(channelNo);
                        MUTEX_LOCK(recordSession[channelNo].dataMutex);
                        recordSession[channelNo].streamType = GET_STREAM_TYPE(tmpCamIdx);
                        readPos = (recordSession[channelNo].preAlrmRcrdStrm == FALSE) ? CI_READ_LATEST_FRAME : CI_READ_OLDEST_FRAME;
                        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

                        //Init session with camera interface
                        InitStreamSession(tmpCamIdx, CI_STREAM_CLIENT_RECORD,readPos);

                        // start Record stream from camera interface
                        if (StartStream(tmpCamIdx, startStreamCallback, CI_STREAM_CLIENT_RECORD) != CMD_SUCCESS)
                        {
                            if(recordSession[channelNo].recordStatus != RECORD_OFF)
                            {
                                recordSession[channelNo].recordStatus = RECORD_OFF_WAIT;
                            }

                            MUTEX_LOCK(recordSession[channelNo].dataMutex);
                            recordType = recordSession[channelNo].recordType;
                            if(recordSession[channelNo].recordFailReason != EVNT_REC_STOP_VIDEO_LOSS)
                            {
                                recordSession[channelNo].recordFailReason = EVNT_REC_STOP_VIDEO_LOSS;
                                MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
                                writeRecordingEvent(channelNo, recordType, REC_FAIL, EVNT_REC_STOP_VIDEO_LOSS);
                                CameraEventNotify(channelNo, RECORDING_FAIL, ACTIVE);
                            }
                            else
                            {
                                MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
                                for(record = 0; record < MAX_RECORD; record++)
                                {
                                    if (GET_BIT(recordType, record))
                                    {
                                        recordSession[channelNo].recordHeathState[record] = REC_FAIL;
                                    }
                                }
                            }
                        }
                    }

                    if(recordSession[channelNo].recordStatus == RECORD_ON_WAIT)
                    {
                        if (TRUE == recordOnWaitStatus[channelNo])
                        {
                            recordSession[channelNo].recordStatus = RECORD_ON;
                            recordSession[channelNo].recordFailReason = MAX_EVNT_REC_STOP;
                            DPRINT(RECORD_MANAGER, "record fsm: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_ON]);

                            MUTEX_LOCK(recordSession[channelNo].dataMutex);
                            recordType = recordSession[channelNo].recordType;
                            recordSession[channelNo].recordFailReason = MAX_EVNT_REC_STOP;
                            MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

                            writeRecordingEvent(channelNo, recordType, REC_START, MAX_EVNT_REC_STOP);
                            CameraEventNotify(channelNo, RECORDING_FAIL, INACTIVE);
                            recordOnWaitStatus[channelNo] = FALSE;
                        }
                        else
                        {
                            recordWriterStatus[channelNo] = FALSE;
                        }
                    }
                    MUTEX_UNLOCK(frameWrCondMutex);

                    if(recordSession[channelNo].recordStatus == RECORD_ON)
                    {
                        EventDetectFunc(channelNo, RECORDING_START, ACTIVE);
                    }
                }
                break;

                case RECORD_ON:
                {
                    writeFrame = FALSE;
                    MUTEX_LOCK(recordSession[channelNo].dataMutex);
                    recordType = recordSession[channelNo].recordType;
                    /** Create Index for main string or sub string based on the configuration */
                    tmpCamIdx = (channelNo + (recordChannelMax * recordSession[channelNo].streamType));
                    adaptiveRecordingF = recordSession[channelNo].adaptiveRecF;
                    MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
                    MUTEX_UNLOCK(frameWrCondMutex);

                    if (FAIL == GetDmBufferState(channelNo))
                    {
                        MUTEX_LOCK(frameWrCondMutex);
                        if (TRUE == recordOffWaitStatus[channelNo])
                        {
                            recordSession[channelNo].recordStatus = RECORD_OFF_WAIT;
                            recordOffWaitStatus[channelNo] = FALSE;
                        }
                        else
                        {
                            if(recordSession[channelNo].recordStatus == RECORD_ON)
                            {
                                recordWriterStatus[channelNo] = FALSE;
                            }
                        }
                        MUTEX_UNLOCK(frameWrCondMutex);
                        break;
                    }

                    pendFrame = GetNextFrame(tmpCamIdx, CI_STREAM_CLIENT_RECORD, &streamInfo, &streadmDataPtr, &streamLen);
                    if(pendFrame >= 1)
                    {
                        /** Alarm record flag present in record type when Alarm recording is enabled and any motion event is occured */
                        frameSkipF = FALSE;
                        if((GET_BIT(recordType, ALARM_RECORD) == 0) && (GET_BIT(recordType, COSEC_RECORD) == 0))
                        {
                            if (adaptiveRecordingF == TRUE)
                            {
                                if (streamInfo->streamType == STREAM_TYPE_VIDEO)
                                {
                                    if (streamInfo->streamPara.videoStreamType != I_FRAME)
                                    {
                                        frameSkipF = TRUE;
                                    }
                                }
                                else
                                {
                                    frameSkipF = TRUE;
                                }
                            }
                        }

                        if(frameSkipF == FALSE)
                        {
                            prevFrameTime = recordSession[channelNo].localTime.totalSec;
                            timeDiff = ((INT32)(prevFrameTime - streamInfo->localTime.totalSec));

                            if((recordSession[channelNo].preRecdStrmSkip == YES) && ((timeDiff >= 5) || (timeDiff <= TWO_FRAME_TIME_DIFF)))
                            {
                                EPRINT(RECORD_MANAGER, "frame time was updated: [camera=%d], [prev=%d], [cur=%d], [diff=%d]",
                                       channelNo, prevFrameTime, streamInfo->localTime.totalSec, timeDiff);

                                /* Time of current frame was older than previous frame. so here we should
                                 * stop the current working file and start recording into new file */
                                StopRecordSession(channelNo);
                                if (StartRecordSession(channelNo) == FAIL)
                                {
                                    EPRINT(RECORD_MANAGER, "fail to start record session: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_ON]);
                                }

                                if(GetDmBufferState(channelNo) == SUCCESS)
                                {
                                    writeFrame = TRUE;
                                }
                                else
                                {
                                    recordSession[channelNo].localTime.totalSec = streamInfo->localTime.totalSec;
                                }
                            }
                            else
                            {
                                writeFrame = TRUE;
                                if(recordSession[channelNo].preRecdStrmSkip == NO)
                                {
                                    preRecordTime = 0;
                                    if (GET_BIT(recordType, ALARM_RECORD))
                                    {
                                        ReadSingleAlarmRecordConfig(channelNo, &alarmRecordCfg);
                                        preRecordTime = alarmRecordCfg.preRecordTime;
                                    }
                                    else if (GET_BIT(recordType, COSEC_RECORD))
                                    {
                                        ReadSingleCosecPreRecConfig(channelNo, &cosecRecParam);
                                        preRecordTime = (cosecRecParam.enable == ENABLE) ? cosecRecParam.preRecDuration : 0;
                                    }

                                    if (preRecordTime > 0)
                                    {
                                        timeDiff = ((INT32)(prevFrameTime - streamInfo->localTime.totalSec));
                                        if(timeDiff <= (INT32)preRecordTime)
                                        {
                                            recordSession[channelNo].preRecdStrmSkip = YES;
                                            DPRINT(RECORD_MANAGER, "pre-record frame skip: [camera=%d], [prev=%d], [cur=%d], [diff=%dsec]",
                                                   channelNo, prevFrameTime, streamInfo->localTime.totalSec, timeDiff);
                                        }
                                        else
                                        {
                                            writeFrame = FALSE;
                                        }
                                    }
                                    else
                                    {
                                        if(prevFrameTime <= streamInfo->localTime.totalSec)
                                        {
                                            recordSession[channelNo].preRecdStrmSkip = YES;
                                            DPRINT(RECORD_MANAGER, "pre-record frame skip: [camera=%d], [prev=%d], [cur=%d], [diff=%dsec]",
                                                   channelNo, prevFrameTime, streamInfo->localTime.totalSec, (prevFrameTime - streamInfo->localTime.totalSec));
                                        }
                                        else
                                        {
                                            writeFrame = FALSE;
                                        }
                                    }
                                }
                            }

                            if(writeFrame == TRUE)
                            {
                                metaDataInfo.channelNo = channelNo;
                                metaDataInfo.mediaType = streamInfo->streamType;
                                metaDataInfo.eventType = recordType;
                                metaDataInfo.localTime.totalSec = streamInfo->localTime.totalSec;
                                metaDataInfo.localTime.mSec = streamInfo->localTime.mSec;
                                metaDataInfo.fps = (UINT16)streamInfo->streamPara.sampleRate;
                                metaDataInfo.codecType = streamInfo->streamPara.streamCodecType;
                                metaDataInfo.resolution = streamInfo->streamPara.resolution;
                                metaDataInfo.vop = streamInfo->streamPara.videoStreamType;
                                metaDataInfo.noOfRefFrame = streamInfo->streamPara.noOfRefFrame;

                                //write media data as well as metadata information
                                if(WriteMediaFrame(streadmDataPtr, streamLen, channelNo, &metaDataInfo, &errorCode) == FAIL)
                                {
                                    HandleDiskError(channelNo, errorCode);

                                    /** Clear error code after processing error */
                                    errorCode = INVALID_ERROR_CODE;
                                }

                                // Update prevFrmTime from current frame time
                                recordSession[channelNo].localTime.totalSec = streamInfo->localTime.totalSec;
                                recordSession[channelNo].localTime.mSec = streamInfo->localTime.mSec;
                            }
                        }
                    }

                    MUTEX_LOCK(frameWrCondMutex);
                    if (TRUE == recordOffWaitStatus[channelNo])
                    {
                        recordSession[channelNo].recordStatus = RECORD_OFF_WAIT;
                        recordOffWaitStatus[channelNo] = FALSE;
                        MUTEX_UNLOCK(frameWrCondMutex);
                    }
                    else
                    {
                        MUTEX_UNLOCK(frameWrCondMutex);
                        if(pendFrame < 2)
                        {
                            MUTEX_LOCK(frameWrCondMutex);
                            if(recordSession[channelNo].recordStatus == RECORD_ON)
                            {
                                recordWriterStatus[channelNo] = FALSE;
                            }
                            MUTEX_UNLOCK(frameWrCondMutex);
                        }
                    }
                }
                break;

                case RECORD_OFF_WAIT:
                {
                    MUTEX_UNLOCK(frameWrCondMutex);
                    DPRINT(RECORD_MANAGER, "record fsm: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_OFF_WAIT]);

                    StopRecordSession(channelNo);
                    StopStream(getRecordChannelNo(channelNo), CI_STREAM_CLIENT_RECORD);

                    MUTEX_LOCK(frameWrCondMutex);
                    MUTEX_LOCK(recordSession[channelNo].dataMutex);
                    recordSession[channelNo].alrmRecordCnt = 0;
                    recordSession[channelNo].recordType = 0;
                    MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
                    recordSession[channelNo].recordStatus = RECORD_OFF;
                    recordWriterStatus[channelNo] = FALSE;
                    MUTEX_UNLOCK(frameWrCondMutex);
                }
                break;

                case RECORD_RESTART_CLEANUP:
                {
                    MUTEX_LOCK(recordSession[channelNo].dataMutex);
                    recordType = recordSession[channelNo].recordType;
                    recStopRes = recordSession[channelNo].recordFailReason;
                    MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
                    MUTEX_UNLOCK(frameWrCondMutex);
                    DPRINT(RECORD_MANAGER, "record fsm: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_RESTART_CLEANUP]);

                    if(StopRecordSession(channelNo) == SUCCESS)
                    {
                        MUTEX_LOCK(frameWrCondMutex);
                        writeRecordingEvent(channelNo, recordType, REC_FAIL, recStopRes);
                        CameraEventNotify(channelNo, RECORDING_FAIL, ACTIVE);
                    }
                    else
                    {
                        EPRINT(RECORD_MANAGER, "fail to stop record session: [camera=%d]", channelNo);
                        MUTEX_LOCK(frameWrCondMutex);
                    }

                    /* If record state change during stop record session */
                    if (recordSession[channelNo].recordStatus != RECORD_RESTART_CLEANUP)
                    {
                        WPRINT(RECORD_MANAGER, "intermediate state changed in record restart cleanup: [camera=%d], [state=%s]",
                               channelNo, recordStateStr[recordSession[channelNo].recordStatus]);
                    }
                    else
                    {
                        recordWriterStatus[channelNo] = FALSE;
                        if (TRUE == recordRestartWaitStatus[channelNo])
                        {
                            recordRestartWaitStatus[channelNo] = FALSE;
                            recordSession[channelNo].recordStatus = RECORD_RESTART_INIT;
                            recordWriterStatus[channelNo] = TRUE;
                        }
                    }
                    MUTEX_UNLOCK(frameWrCondMutex);
                }
                break;

                case RECORD_RESTART_INIT:
                {
                    MUTEX_LOCK(recordSession[channelNo].dataMutex);
                    recordType = recordSession[channelNo].recordType;
                    MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
                    DPRINT(RECORD_MANAGER, "record fsm: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_RESTART_INIT]);

                    if(StartRecordSession(channelNo) == SUCCESS)
                    {
                        recordSession[channelNo].recordFailReason = MAX_EVNT_REC_STOP;
                        writeRecordingEvent(channelNo, recordType, REC_START, MAX_EVNT_REC_STOP);
                        EventDetectFunc(channelNo, RECORDING_START, ACTIVE);
                        CameraEventNotify(channelNo, RECORDING_FAIL, INACTIVE);
                        recordSession[channelNo].recordStatus = RECORD_ON;
                    }
                    else
                    {
                        EPRINT(RECORD_MANAGER, "fail to restart record session: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_RESTART_INIT]);
                        recordSession[channelNo].recordStatus = RECORD_RESTART_CLEANUP;
                    }
                    recordWriterStatus[channelNo] = FALSE;
                    MUTEX_UNLOCK(frameWrCondMutex);
                }
                break;

                case RECORD_DRIVE_SWITCH:
                {
                    MUTEX_LOCK(recordSession[channelNo].dataMutex);
                    recordType = recordSession[channelNo].recordType;
                    MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
                    DPRINT(RECORD_MANAGER, "record fsm: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_DRIVE_SWITCH]);

                    StopRecordSession(channelNo);
                    if (StartRecordSession(channelNo) == FAIL)
                    {
                        EPRINT(RECORD_MANAGER, "fail to start record session: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_DRIVE_SWITCH]);
                    }

                    if(recordSession[channelNo].recordStatus == RECORD_DRIVE_SWITCH)
                    {
                        recordWriterStatus[channelNo] = FALSE;
                        recordSession[channelNo].recordStatus = RECORD_ON;
                    }
                    MUTEX_UNLOCK(frameWrCondMutex);
                }
                break;

                case RECORD_STREAM_SWITCH:
                {
                    MUTEX_LOCK(recordSession[channelNo].dataMutex);
                    recordType = recordSession[channelNo].recordType;
                    if(recordSession[channelNo].preAlrmRcrdStrm == FALSE)
                    {
                        readPos = CI_READ_LATEST_FRAME;
                    }
                    else
                    {
                        readPos = CI_READ_OLDEST_FRAME;
                    }
                    MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

                    /* Get previous camera index to stop previous stream type */
                    tmpCamIdx = getPrevStreamRecordChannelNo(channelNo);
                    DPRINT(RECORD_MANAGER, "record fsm: [camera=%d], [state=%s], [streamType=%d]",
                           channelNo, recordStateStr[RECORD_STREAM_SWITCH], GET_STREAM_TYPE(tmpCamIdx));
                    StopRecordSession(channelNo);
                    StopStream(tmpCamIdx, CI_STREAM_CLIENT_RECORD);

                    if(StartRecordSession(channelNo) == SUCCESS)
                    {
                        /* Get camera index to start with new stream type */
                        tmpCamIdx = getRecordChannelNo(channelNo);
                        DPRINT(RECORD_MANAGER, "record stream switch success: [camera=%d], [streamType=%d]", channelNo, GET_STREAM_TYPE(tmpCamIdx));
                        InitStreamSession(tmpCamIdx, CI_STREAM_CLIENT_RECORD, readPos);
                        recordSession[channelNo].recordStatus = RECORD_ON_WAIT;

                        MUTEX_LOCK(recordSession[channelNo].dataMutex);
                        recordSession[channelNo].streamType = GET_STREAM_TYPE(tmpCamIdx);
                        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);

                        recordWriterStatus[channelNo] = FALSE;
                        // start Record stream from camera interface
                        if (StartStream(tmpCamIdx, startStreamCallback, CI_STREAM_CLIENT_RECORD) != CMD_SUCCESS)
                        {
                            stopAllRecording = TRUE;
                        }
                    }
                    else
                    {
                        EPRINT(RECORD_MANAGER, "fail to start record session: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_STREAM_SWITCH]);
                        recordWriterStatus[channelNo] = FALSE;
                        stopAllRecording = TRUE;
                    }

                    if(stopAllRecording == TRUE)
                    {
                        stopAllRecording = FALSE;
                        for(recType = MANUAL_RECORD; recType < MAX_RECORD; recType++)
                        {
                            stopInternalRecord(channelNo, recType, FALSE);
                        }
                    }
                    MUTEX_UNLOCK(frameWrCondMutex);
                }
                break;

                default:
                {
                    EPRINT(RECORD_MANAGER, "unhandle state: [camera=%d], [state=%d]", channelNo, recordSession[channelNo].recordStatus);
                }
                /* FALLS THROUGH */
                case RECORD_OFF:
                {
                    recordWriterStatus[channelNo] = FALSE;
                    MUTEX_UNLOCK(frameWrCondMutex);
                }
                break;
            }
        }
    }

    for(channelNo = 0; channelNo < recordChannelMax; channelNo++)
    {
        MUTEX_LOCK(recordSession[channelNo].dataMutex);
        StopRecordSession(channelNo);
        initRecSessionData(channelNo);
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
    }

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is timer callback function. It will execute every second and wakeup the recordScheduler
 *          for start and stop schedule recording.
 * @param   data
 */
static void checkTimeSlotForRecording(UINT32 data)
{
    BOOL						status = FALSE;
    UINT8						channelCnt = 0;
    struct tm 					curTime = { 0 };
    COSEC_REC_PARAM_CONFIG_t	cosecRecParam;
    CAMERA_CONFIG_t				camConfig;
    UINT8						recFailCnt = 0, totalEnableCamera = 0;
    UINT8						recType = 0;
    CHAR						eventDetail[MAX_EVENT_DETAIL_SIZE];
    static BOOL                 recFailForCamera = FALSE;
    static BOOL                 recFailForAllCamera = FALSE;

    GetLocalTimeInBrokenTm(&curTime);
    for(channelCnt = 0; channelCnt < getMaxCameraForCurrentVariant(); channelCnt++)
    {
        ReadSingleCameraConfig(channelCnt, &camConfig);
        if(camConfig.camera == ENABLE)
        {
            /* Calculate no. of enabled cameras */
            totalEnableCamera++;

            /* Check the current time comes under the schedule time or not */
            if (valSchdlRecordTime(channelCnt, &curTime) == SUCCESS)
            {
                /* Starts the schedule record if already not running, if schedule recording is already running then nothing will be done */
                startInternalRecord(channelCnt, SCHEDULE_RECORD, DEFAULT_REC_DURATION);
            }
            else
            {
                stopInternalRecord(channelCnt, SCHEDULE_RECORD, FALSE);
            }

            status = valAdaptiveRecordTime(channelCnt, &curTime);
            MUTEX_LOCK(recordSession[channelCnt].dataMutex);
            recordSession[channelCnt].adaptiveRecF = status;
            MUTEX_UNLOCK(recordSession[channelCnt].dataMutex);
        }

        ReadSingleCosecPreRecConfig(channelCnt, &cosecRecParam);
        if(cosecRecParam.enable == ENABLE)
        {
            entPreCosecRecStrm(channelCnt);
        }
        else
        {
            exitPreCosecRecStrm(channelCnt);
        }

        for(recType = MANUAL_RECORD; recType < MAX_RECORD; recType++)
        {
            if(recordSession[channelCnt].recordHeathState[recType] != REC_FAIL)
            {
                continue;
            }

            if(camConfig.camera == ENABLE)
            {
                recFailCnt++;
                break;
            }

            recordSession[channelCnt].recordHeathState[recType] = REC_STOP;
        }
    }

    if((recFailCnt > 0) && (recFailCnt < totalEnableCamera) && (recFailForCamera == FALSE))
    {
        recFailForCamera = TRUE;
        recFailForAllCamera = FALSE;
        SetSystemStatusLed(SYS_RECORDING_FAIL, ON);
    }
    else if((recFailCnt == totalEnableCamera) && (recFailForAllCamera == FALSE) && (totalEnableCamera != 0))
    {
        recFailForAllCamera = TRUE;
        recFailForCamera = FALSE;
        SetSystemStatusLed(SYS_RECORDING_FAIL_FOR_ALL, ON);
    }
    else if((recFailCnt == 0) && ((recFailForCamera == TRUE) || (recFailForAllCamera == TRUE)))
    {
        if(recFailForCamera == TRUE)
        {
            recFailForCamera = FALSE;
            SetSystemStatusLed(SYS_RECORDING_FAIL, OFF);
            SetSystemStatusLed(SYS_RECORDING_FAIL_FOR_ALL, OFF);
        }
        else if(recFailForAllCamera == TRUE)
        {
            recFailForAllCamera = FALSE;
            SetSystemStatusLed(SYS_RECORDING_FAIL, OFF);
            SetSystemStatusLed(SYS_RECORDING_FAIL_FOR_ALL, OFF);
        }
    }

    if((recFailCnt > 0) && (isRecInitDone == TRUE))
    {
        snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", recFailCnt);
        WriteEvent(LOG_SYSTEM_EVENT, LOG_RECORDING_FAIL, eventDetail, "SYSTEM", EVENT_ALERT);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   recordInitHandling
 * @param   data
 * @return
 */
static void recordInitHandling(UINT32 data)
{
    DeleteTimer(&recInitHandle);
    isRecInitDone = TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This fuction checks whether the given time falls into the schedule record time. If time
 *          was not between the schedule record time then it will return FAIL else
 * @param   channelNo
 * @param   brokenTime
 * @return
 */
static BOOL valSchdlRecordTime(UINT8 channelNo, struct tm *brokenTime)
{
    UINT8                       cnt;
    TIME_HH_MM_t                curTime;
    SCHEDULE_RECORD_CONFIG_t    scheduleRecordConfig;

    curTime.hour = brokenTime->tm_hour;
    curTime.minute = brokenTime->tm_min;

    ReadSingleScheduleRecordConfig(channelNo, &scheduleRecordConfig);
    if(scheduleRecordConfig.scheduleRecording == DISABLE)
    {
        return FAIL;
    }

    if (ENABLE == GET_BIT(scheduleRecordConfig.dailyRecord[brokenTime->tm_wday].recordEntireDay, RECORD_SCHEDULE_ENTIRE_DAY))
    {
        return SUCCESS;
    }

    // check every daily schedule record
    for(cnt = 0; cnt < MAX_DAILY_SCHEDULE; cnt++)
    {
        if(IsGivenTimeInWindow(curTime, scheduleRecordConfig.dailyRecord[brokenTime->tm_wday].period[cnt].startTime,
                               scheduleRecordConfig.dailyRecord[brokenTime->tm_wday]. period[cnt].endTime) == YES)
        {
            return SUCCESS;
        }
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief valAdaptiveRecordTime - This fuction checks whether the given time falls into the schedule
 *                                record time. If time was not between the schedule record time then
 *                                it will return FAIL else return SUCCESS.
 * @param channelNo
 * @param brokenTime
 * @return
 */
static BOOL valAdaptiveRecordTime(UINT8 channelNo, struct tm *brokenTime)
{
    UINT8                       cnt;
    TIME_HH_MM_t                curTime;
    SCHEDULE_RECORD_CONFIG_t    scheduleRecordConfig;
    UINT8                       weekDay = brokenTime->tm_wday;

    curTime.hour = brokenTime->tm_hour;
    curTime.minute = brokenTime->tm_min;

    /* check if adaptive is enable for entire day */
    ReadSingleScheduleRecordConfig(channelNo, &scheduleRecordConfig);
    if (ENABLE == GET_BIT(scheduleRecordConfig.dailyRecord[weekDay].recordEntireDay, RECORD_ADAPTIVE_ENTIRE_DAY))
    {
        return ENABLE;
    }

    /* check if adaptive is enable for any period */
    for(cnt = 0; cnt < MAX_DAILY_SCHEDULE; cnt++)
    {
        /* check if current time is present in any period */
        if(IsGivenTimeInWindow(curTime, scheduleRecordConfig.dailyRecord[weekDay].period[cnt].startTime,
                               scheduleRecordConfig.dailyRecord[weekDay].period[cnt].endTime) == YES)
        {
            /* if current time is present in period then check if adaptive flag for this period is enable or not */
            if (ENABLE == GET_BIT(scheduleRecordConfig.dailyRecord[weekDay].recordEntireDay, (RECORD_ADAPTIVE_PERIOD_1 + cnt)))
            {
                return ENABLE;
            }
        }
    }

    return DISABLE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives health status of recording for each channel and all type of recording.
 * @param   channelNo
 * @param   recType
 * @return
 */
BOOL GetRecordStatus(UINT8 channelNo, UINT8 recType)
{
    if (channelNo >= getMaxCameraForCurrentVariant())
    {
        EPRINT(RECORD_MANAGER, "invld camera index: [camera=%d]", channelNo);
        return REC_STOP;
    }

    if (recType >= MAX_RECORD)
    {
        EPRINT(RECORD_MANAGER, "invld record type: [camera=%d], [recType=%d]", channelNo, recType);
        return REC_STOP;
    }

    return recordSession[channelNo].recordHeathState[recType];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives health status of Adaptive recording for each channel.
 * @param   channelNo
 * @param   value
 * @return
 */
BOOL GetAdaptiveRecordStatus(UINT8 channelNo, UINT8 value)
{
    UINT8 recType;

    if (channelNo >= getMaxCameraForCurrentVariant())
    {
        return REC_FAIL;
    }

    if ((recordSession[channelNo].adaptiveRecF == DISABLE) || (INACTIVE == GetCamEventStatus(channelNo, CONNECTION_FAILURE)))
    {
        return REC_STOP;
    }

    for(recType = MANUAL_RECORD; recType < MAX_RECORD; recType++)
    {
        if(recordSession[channelNo].recordHeathState[recType] == REC_START)
        {
            return REC_START;
        }

        if(recordSession[channelNo].recordHeathState[recType] == REC_FAIL)
        {
            return REC_FAIL;
        }
    }

    return REC_STOP;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will be called on camera configuration change and takes appropriate action
 *          for the changes.
 * @param   newCameraConfig
 * @param   oldCameraConfig
 * @param   cameraIndex
 */
void RMConfigNotify(CAMERA_CONFIG_t newCameraConfig, CAMERA_CONFIG_t *oldCameraConfig, UINT8 cameraIndex)
{
    struct tm brokenTime = { 0 };

    if (newCameraConfig.recordingStream == oldCameraConfig->recordingStream)
    {
        return;
    }

    GetLocalTimeInBrokenTm(&brokenTime);

    //it is ignored for other types than schedule, requirement should be rechecked
    if (FAIL == valSchdlRecordTime(cameraIndex, &brokenTime))
    {
        return;
    }

    MUTEX_LOCK(frameWrCondMutex);
    if (TRUE == recordWriterStatus[cameraIndex])
    {
        STATE_OVERRIDE_PRINT(cameraIndex, recordStateStr[recordSession[cameraIndex].recordStatus], recordStateStr[RECORD_STREAM_SWITCH]);
    }

    recordSession[cameraIndex].recordStatus = RECORD_STREAM_SWITCH;
    recordWriterStatus[cameraIndex] = TRUE;
    pthread_cond_signal(&frameWrCondSignal);
    MUTEX_UNLOCK(frameWrCondMutex);
    DPRINT(RECORD_MANAGER, "record session state: [camera=%d], [state=%s]", cameraIndex, recordStateStr[RECORD_STREAM_SWITCH]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Below api restarts the recording session of the input channel. If no recording is running
 *          then nothing will be done
 * @param   channelNo - Channel for which session needs to be restart
 * @param   errorCode - Error code number due to which session needs to be restart
 */
void RestartRecSession(UINT8 channelNo, UINT32 errorCode)
{
    CHAR eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR advDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    /** validate parameters */
    if (channelNo >= getMaxCameraForCurrentVariant())
    {
        return;
    }

    MUTEX_LOCK(frameWrCondMutex);
    MUTEX_LOCK(recordSession[channelNo].dataMutex);

    /** check recording is ON */
    if(recordSession[channelNo].recordStatus != RECORD_ON)
    {
        MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
        MUTEX_UNLOCK(frameWrCondMutex);
        return;
    }

    /** check any processing is left or not */
    if (TRUE == recordWriterStatus[channelNo])
    {
        STATE_OVERRIDE_PRINT(channelNo, recordStateStr[recordSession[channelNo].recordStatus], recordStateStr[RECORD_DRIVE_SWITCH]);
    }

    /** set recording state to record switch to restart the recording session */
    recordSession[channelNo].recordStatus = RECORD_DRIVE_SWITCH;

    /** set channel writer processing */
    recordWriterStatus[channelNo] = TRUE;

    /** give signal to record manager thread */
    pthread_cond_signal(&frameWrCondSignal);

    snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "Camera %02d", GET_CAMERA_NO(channelNo));
    if (INVALID_ERROR_CODE == errorCode)
    {
        advDetail[0] = '\0';
    }
    else
    {
        snprintf(advDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", strerror(errorCode));
    }

    WriteEvent(LOG_SYSTEM_EVENT, LOG_RECORDING_RESTART, eventDetail, advDetail, EVENT_ALERT);

    MUTEX_UNLOCK(recordSession[channelNo].dataMutex);
    MUTEX_UNLOCK(frameWrCondMutex);
    DPRINT(RECORD_MANAGER, "record session state: [camera=%d], [state=%s]", channelNo, recordStateStr[RECORD_DRIVE_SWITCH]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get camera index based on recording stream type configuration
 * @param   channelNo
 * @return  Record stream camera index
 */
static UINT8 getRecordChannelNo(UINT8 channelNo)
{
    CAMERA_CONFIG_t cameraCfg;

    ReadSingleCameraConfig(channelNo, &cameraCfg);
    return (channelNo + (getMaxCameraForCurrentVariant() * cameraCfg.recordingStream));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get camera index based on previous recording stream type configuration. It is useful
 *          when stream type is changed
 * @param   channelNo
 * @return  Record stream camera index
 */
static UINT8 getPrevStreamRecordChannelNo(UINT8 channelNo)
{
    CAMERA_CONFIG_t cameraCfg;

    /* Change current stream type to get the previous recording channel */
    ReadSingleCameraConfig(channelNo, &cameraCfg);
    cameraCfg.recordingStream = (cameraCfg.recordingStream == MAIN_STREAM) ? SUB_STREAM : MAIN_STREAM;
    return (channelNo + (getMaxCameraForCurrentVariant() * cameraCfg.recordingStream));
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
