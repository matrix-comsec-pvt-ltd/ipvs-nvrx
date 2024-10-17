//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		InputOutput.c
@brief      In this module we provide the functionality with which the user can configure the Sensors
            with the NVR system. Three digital input-output pins are available, Two for INPUT and one
            for OUTPUT. Two Input pins are for SENSOR INPUT and one output pin for ALARM OUTPUT. The
            modes of input sensors can be configured either Normal-Open or Normal-Close with de-bounce
            time, to avoid false triggering. The mode of output sensor can be configured as interlock
            (the respective output sensor shall get activated by an event trigger, and shall be normalized
            once that event is normal) or pulse (the respective output sensor shall get activated by
            an event trigger, but shall be normalized after user specified time period, starting from
            the time of activation be an event; irrespective of the state of that event).
            This module provides APIs which configures the Input and Output sensors as specified by
            user and API which sends the signal to the module or performs an action when event is
            generated related to IO.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/ioctl.h>

/* Application Includes */
#include "Utils.h"
#include "InputOutput.h"
#include "SysTimer.h"
#include "EventHandler.h"
#include "DebugLog.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define INPUT_PIN_STATUS_CNT    4   // 4 Samples
#define INIT_LED_BUZZER_CYCLE   4   // Second
#define	RESET_BTN_TIME          4   // Second
#define GET_INDEX_NO(x)         (x + 1)
#define CPU_TEMP_ADV_DETAIL		"Abnormal CPU Temperature"
#define	CPU_TEMPERATURE_FILE    "/sys/class/thermal/thermal_zone0/temp"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    #if defined(RK3568_NVRL) || defined(RK3588_NVRH)
    NVR_XP2_BOARD_V1R0 = 0,
    NVR_XP2_BOARD_V1R1 = 1,
    #else
    NVR_X_BOARD_V1R2 = 0,
    NVR_X_BOARD_V1R3 = 1,
    NVR_X_BOARD_V2R2 = 2,
    #endif
    NVR_X_BOARD_VERSION_MAX
}NVR_X_BOARD_VERSION_e;

typedef enum
{
    #if defined(RK3568_NVRL)
    NVR_MODEL_0801X     = 0,
    NVR_MODEL_1601X     = 1,
    NVR_MODEL_1602X     = 2,
    NVR_MODEL_0801XS    = 3,
    NVR_MODEL_1601XS    = 4,
    NVR_MODEL_0401XS    = 5,
    #elif defined(RK3588_NVRH)
    #if defined(OEM_JCI)
    NVR_MODEL_HRIN_1208 = 0,
    NVR_MODEL_HRIN_2808 = 1,
    NVR_MODEL_HRIN_4808 = 2,
    NVR_MODEL_HRIN_6408 = 3,
    #else
    NVR_MODEL_3202X     = 0,
    NVR_MODEL_3204X     = 1,
    NVR_MODEL_6404X     = 2,
    NVR_MODEL_6408X     = 3,
    NVR_MODEL_9608X     = 4,
    #endif
    #else
    NVR_MODEL_0801X     = 0,
    NVR_MODEL_1601X     = 1,
    NVR_MODEL_1602X     = 2,
    NVR_MODEL_3202X     = 3,
    NVR_MODEL_3204X     = 4,
    NVR_MODEL_6404X     = 5,
    NVR_MODEL_6408X     = 6,
    #endif
    NVR_MODEL_MAX
}NVR_MODEL_e;

/* Input pins on board */
typedef enum
{
    INPUT_SENSOR_1 = 0,
    INPUT_SENSOR_2,
    INPUT_SENSOR_MAX,
    INPUT_RESET_BUTTON = INPUT_SENSOR_MAX,
    INPUT_MAX
}INPUT_PIN_e;

/* Output pins on board */
typedef enum
{
    OUTPUT_ALARM_1 = 0,
    OUTPUT_ALARM_MAX
}OUTPUT_PIN_e;

typedef struct
{
    UINT8   boardVersion;
    BOOL    isMultiplierAvail;
    BOOL    isExtRtcAvail;
}NVR_BOARD_INFO_t;

typedef struct
{
    UINT8           status;
    UINT8           runCnt;
    TIMER_HANDLE    alarmTimerHandle;
    pthread_mutex_t alarmAccessMutex;
}ALARM_RUNING_PARA_t;

// Input pins current status
typedef struct
{
    BOOL            inputPinStatus; // Physical Input Status
    UINT32          statusHoldCnt;  // Physical pin status hold count
}INPUT_MAP_t;

//#################################################################################################
// @EXTERN
//#################################################################################################
extern void SensorEventNotify(UINT8 sensorIndex, BOOL sensorActive);

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static ALARM_RUNING_PARA_t  alarmRunPara[OUTPUT_ALARM_MAX];
static INPUT_MAP_t			inputStatusMap[INPUT_MAX];
static BOOL					currSensorState[INPUT_SENSOR_MAX];

//Timer handle for check IO status, init LED+Buzzer, Reset button and sensor input
static TIMER_HANDLE 		checkIOStatusTimerHandle = INVALID_TIMER_HANDLE;
static TIMER_HANDLE	 		initLedBuzTimerHandle = INVALID_TIMER_HANDLE;
static TIMER_HANDLE	 		resetBtnTimerHandle = INVALID_TIMER_HANDLE;
static TIMER_HANDLE	 		sensorInputTimerHandle[INPUT_SENSOR_MAX];

//Mutex to synchronize access of IO Cfg
static pthread_mutex_t		ioAccessMutex[INPUT_SENSOR_MAX];
static pthread_mutex_t      buzzerStatusMutex;
static BOOL                 buzzerStatus = OFF;
static NVR_BOARD_INFO_t     boardInfo;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void stopAlarm(UINT32 alarmIndex);
//-------------------------------------------------------------------------------------------------
static void initLedBuzTimerCB(UINT32 arg);
//-------------------------------------------------------------------------------------------------
static void checkIOStatus(UINT32 arg);
//-------------------------------------------------------------------------------------------------
static void dispatchIOStatus(INPUT_PIN_e inputNo, BOOL status);
//-------------------------------------------------------------------------------------------------
static void resetBtnTimerCB(UINT32 arg);
//-------------------------------------------------------------------------------------------------
static void changeSensorInputCB(UINT32 sensorIndex);
//-------------------------------------------------------------------------------------------------
static void setBuzzerStatus(BOOL status);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Init digital input-output parameters and start related timers
 */
void InitIO(void)
{
    UINT8			ioNum;
 	TIMER_INFO_t	initLedBuzTimer;

    for(ioNum = 0; ioNum < OUTPUT_ALARM_MAX; ioNum++)
	{
        alarmRunPara[ioNum].status = 0;
        alarmRunPara[ioNum].runCnt = 0;
        alarmRunPara[ioNum].alarmTimerHandle = INVALID_TIMER_HANDLE;
        MUTEX_INIT(alarmRunPara[ioNum].alarmAccessMutex, NULL);
	}

    for(ioNum = 0; ioNum < INPUT_SENSOR_MAX; ioNum++)
	{
        MUTEX_INIT(ioAccessMutex[ioNum], NULL);
        currSensorState[ioNum] = INACTIVE;
        sensorInputTimerHandle[ioNum] = INVALID_TIMER_HANDLE;
	}

    for(ioNum = 0; ioNum < INPUT_MAX; ioNum++)
	{
        inputStatusMap[ioNum].inputPinStatus = TRUE;
        inputStatusMap[ioNum].statusHoldCnt = 0;
	}

	checkIOStatusTimerHandle = INVALID_TIMER_HANDLE;
	initLedBuzTimerHandle = INVALID_TIMER_HANDLE;
	resetBtnTimerHandle = INVALID_TIMER_HANDLE;;

    /* Start the init led and buzzer */
	initLedBuzTimer.count = CONVERT_SEC_TO_TIMER_COUNT(1);
	initLedBuzTimer.data = 0;
	initLedBuzTimer.funcPtr = initLedBuzTimerCB;
	StartTimer(initLedBuzTimer, &initLedBuzTimerHandle);

    /* Init buzzer status mutex lock */
    MUTEX_INIT(buzzerStatusMutex, NULL);

	// Set system in normal state
	SetSystemStatusLed(SYS_NORMAL_STATE, ON);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   De init of input-output pins
 */
void DeinitIO(void)
{
    UINT8 sensorNo;

	MuteBuzzer();

    for(sensorNo = 0; sensorNo < INPUT_SENSOR_MAX; sensorNo++)
	{
        DeleteTimer(&sensorInputTimerHandle[sensorNo]);
	}

	DeleteTimer(&resetBtnTimerHandle);
	DeleteTimer(&initLedBuzTimerHandle);
	DeleteTimer(&checkIOStatusTimerHandle);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates the local working copy of the sensor whose configurations have been changed
 * @param   newSensorConfig
 * @param   oldSensorConfig
 * @param   sensorNo
 */
void UpdateSensorConfig(SENSOR_CONFIG_t newSensorConfig, SENSOR_CONFIG_t *oldSensorConfig, UINT8 sensorNo)
{
    CHAR detail[MAX_EVENT_DETAIL_SIZE];

    if ((sensorNo >= INPUT_SENSOR_MAX) || (sensorNo >= GetNoOfSensorInput()))
 	{
        return;
    }

    if (((newSensorConfig.sensorDetect == DISABLE) && (oldSensorConfig->sensorDetect == ENABLE))
            || (newSensorConfig.normalMode != oldSensorConfig->normalMode))
    {
        MUTEX_LOCK(ioAccessMutex[sensorNo]);
        if (currSensorState[sensorNo] == INACTIVE)
        {
            MUTEX_UNLOCK(ioAccessMutex[sensorNo]);
            return;
        }

        currSensorState[sensorNo] = INACTIVE;
        snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_INDEX_NO(sensorNo));
        WriteEvent(LOG_SENSOR_EVENT, LOG_SENSOR_INPUT, detail, oldSensorConfig->name, EVENT_NORMAL);
        SensorEventNotify(sensorNo, INACTIVE);
        MUTEX_UNLOCK(ioAccessMutex[sensorNo]);
        DeleteTimer(&sensorInputTimerHandle[sensorNo]);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates the local working copy of the alarm whose configurations have been changed.
 * @param   newCfg
 * @param   oldCfg
 * @param   alarmNo
 */
void UpdateAlarmConfig(ALARM_CONFIG_t newCfg, ALARM_CONFIG_t *oldCfg, UINT8 alarmNo)
{
 	ALARM_RUNING_PARA_t		tempAlmPara;
 	LOG_EVENT_STATE_e		evtLog = EVENT_NORMAL;
 	CHAR					detail[MAX_EVENT_DETAIL_SIZE];
 	UINT32  				alarmStatus;

    if ((alarmNo >= OUTPUT_ALARM_MAX) || (alarmNo >= GetNoOfAlarmOutput()))
 	{
        return;
    }

    if ((newCfg.activeMode == ALARM_PULSE) && (oldCfg->activeMode == ALARM_INTERLOCK))
    {
        tempAlmPara.status = ALMOUT_CLEAR;
        tempAlmPara.runCnt = 0;
    }
    else if(newCfg.alarmOutput != oldCfg->alarmOutput)
    {
        tempAlmPara.status = ALMOUT_CLEAR;
        tempAlmPara.runCnt = 0;
    }
    else
    {
        return;
    }

    MUTEX_LOCK(alarmRunPara[alarmNo].alarmAccessMutex);
    if (newCfg.alarmOutput == ENABLE)
    {
        if(alarmRunPara[alarmNo].runCnt > 0)
        {
            tempAlmPara.status = ALMOUT_SET;
            tempAlmPara.runCnt = alarmRunPara[alarmNo].runCnt;
            evtLog = EVENT_ACTIVE;
        }
    }
    alarmRunPara[alarmNo].status = tempAlmPara.status;
    alarmRunPara[alarmNo].runCnt = tempAlmPara.runCnt;

    alarmStatus = tempAlmPara.status;
    INT32 gpioDrvFd = open(GPIO_DEVICE_NAME, READ_WRITE_MODE);
    if (gpioDrvFd < 0)
    {
        EPRINT(INPUT_OUTPUT, "failed to open gpio driver: [err=%s]", STR_ERR);
    }
    else
    {
        if(ioctl(gpioDrvFd, MXGPIO_SET_ALMOUT, &alarmStatus) < 0)
        {
            EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_ALMOUT failed: [err=%s]", STR_ERR);
        }

        close(gpioDrvFd);
    }

    snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_INDEX_NO(alarmNo));
    WriteEvent(LOG_ALARM_EVENT, LOG_ALARM_OUTPUT, detail, NULL, evtLog);
    MUTEX_UNLOCK(alarmRunPara[alarmNo].alarmAccessMutex);
    DeleteTimer(&alarmRunPara[alarmNo].alarmTimerHandle);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   The System led status is passed by input parameter and boolean to active or de-active
 *          the status of led. As per system status it also changes buzzer cadance.
 * @param   newSystemLedState
 * @param   action
 * @return
 */
BOOL SetSystemStatusLed(SYSTEM_LED_STATE_e newSystemLedState, BOOL action)
{
	UINT32 				value;
	BUZZER_LED_RESP_t 	buzLedResp;
	BUZZER_LED_CAD_t	buzzerResp[BUZZER_LED_MAX];
	BOOL				retVal = SUCCESS;

    buzLedResp.LedCadResp = NVRX_RESP_NONE;
    buzLedResp.BuzCadResp = NVRX_RESP_NONE;

    INT32 gpioDrvFd = open(GPIO_DEVICE_NAME, READ_WRITE_MODE);
    if(gpioDrvFd < 0)
    {
        EPRINT(INPUT_OUTPUT, "failed to open gpio driver: [err=%s]", STR_ERR);
        return FAIL;
    }

    if(ioctl(gpioDrvFd, MXGPIO_GET_CAD_STS, &buzzerResp) < 0)
    {
        EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_GET_CAD_STS failed: [err=%s]", STR_ERR);
        close(gpioDrvFd);
        return FAIL;
    }

    if (action == ON)
    {
        switch(newSystemLedState)
        {
            case SYS_LED_IDLE_STATE:
                value = NVRX_IDLE_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_INCOMPLETE_VOLUME:
                value = NVRX_STORAGE_INCOMPLETE_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
                    break;
                }

                /* set buzzer status to on */
                setBuzzerStatus(ON);
                break;

            case SYS_STORAGE_ALERT:
                value = NVRX_STORAGE_FULL_ALT_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_FULL_VOL:
                value = NVRX_STORAGE_FULL_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_FORMATTING:
                value = NVRX_FORMAT_DISK_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_FIRMWARE_UPGRADE_INPROCESS:
                value = NVRX_FW_UPG_INPROCESS_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_UPGRADE_FAILED:
                value = NVRX_FW_UPG_FAILED_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
                    break;
                }

                /* set buzzer status to on */
                setBuzzerStatus(ON);
                break;

            case SYS_TRG_EVT_RESP:
                value = NVRX_TRG_EVT_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
                {
                    retVal = FAIL;
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
                    break;
                }

                /* set buzzer status to on */
                setBuzzerStatus(ON);
                break;

            case SYS_BKUP_FULL_RESP:
                value = NVRX_BKUP_FULL_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
                    break;
                }

                /* set buzzer status to on */
                setBuzzerStatus(ON);
                break;

            case SYS_NO_BKUP_RESP:
                value = NVRX_NO_BKUP_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
                    break;
                }

                /* set buzzer status to on */
                setBuzzerStatus(ON);
                break;

            case SYS_NORMAL_STATE:
                value = NVRX_NORMAL_STATE;
                if(ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_RECORDING_FAIL:
                value = NVRX_RECORDING_FAIL;
                if(ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_RECORDING_FAIL_FOR_ALL:
                value = NVRX_RECORDING_FAIL_FOR_ALL;
                if(ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_APPL_INIT_RESP:
            case SYS_NET_CONN_RESP:
            case SYS_NET_DISCONN_RESP:
            default:
                break;
        }
    }
    else
    {
        switch(newSystemLedState)
        {
            case SYS_LED_IDLE_STATE:
                buzLedResp.LedCadResp = NVRX_IDLE_RESP;
                buzLedResp.BuzCadResp = NVRX_IDLE_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE, &buzLedResp) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_INCOMPLETE_VOLUME:
                buzLedResp.LedCadResp = NVRX_STORAGE_INCOMPLETE_RESP;
                buzLedResp.BuzCadResp = NVRX_STORAGE_INCOMPLETE_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE, &buzLedResp) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
                }

                /* set buzzer status to off */
                setBuzzerStatus(OFF);
                break;

            case SYS_STORAGE_ALERT:
                buzLedResp.LedCadResp = NVRX_STORAGE_FULL_ALT_RESP;
                buzLedResp.BuzCadResp = NVRX_STORAGE_FULL_ALT_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE, &buzLedResp) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_FULL_VOL:
                buzLedResp.LedCadResp = NVRX_STORAGE_FULL_RESP;
                buzLedResp.BuzCadResp = NVRX_STORAGE_FULL_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE, &buzLedResp) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_FORMATTING:
                buzLedResp.LedCadResp = NVRX_FORMAT_DISK_RESP;
                buzLedResp.BuzCadResp = NVRX_FORMAT_DISK_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE, &buzLedResp) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_FIRMWARE_UPGRADE_INPROCESS:
                buzLedResp.LedCadResp = NVRX_FW_UPG_INPROCESS_RESP;
                buzLedResp.BuzCadResp = NVRX_FW_UPG_INPROCESS_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE, &buzLedResp) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_UPGRADE_FAILED:
                buzLedResp.LedCadResp = NVRX_FW_UPG_FAILED_RESP;
                buzLedResp.BuzCadResp = NVRX_FW_UPG_FAILED_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE, &buzLedResp) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
                }

                /* set buzzer status to off */
                setBuzzerStatus(OFF);
                break;

            case SYS_TRG_EVT_RESP:
                buzLedResp.LedCadResp = NVRX_TRG_EVT_RESP;
                buzLedResp.BuzCadResp = NVRX_TRG_EVT_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE, &buzLedResp) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
                }

                /* set buzzer status to off */
                setBuzzerStatus(OFF);
                break;

            case SYS_BKUP_FULL_RESP:
                buzLedResp.LedCadResp = NVRX_BKUP_FULL_RESP;
                buzLedResp.BuzCadResp = NVRX_BKUP_FULL_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE, &buzLedResp) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
                }

                /* set buzzer status to off */
                setBuzzerStatus(OFF);
                break;

            case SYS_NO_BKUP_RESP:
                buzLedResp.LedCadResp = NVRX_NO_BKUP_RESP;
                buzLedResp.BuzCadResp = NVRX_NO_BKUP_RESP;
                if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE, &buzLedResp) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
                }

                /* set buzzer status to off */
                setBuzzerStatus(OFF);
                break;

            case SYS_NORMAL_STATE:
                buzLedResp.LedCadResp = NVRX_NORMAL_STATE;
                buzLedResp.BuzCadResp = NVRX_NORMAL_STATE;
                if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE, &buzLedResp) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_RECORDING_FAIL:
                buzLedResp.LedCadResp = NVRX_RECORDING_FAIL;
                buzLedResp.BuzCadResp = NVRX_RECORDING_FAIL;
                if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE,&buzLedResp) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_RECORDING_FAIL_FOR_ALL:
                buzLedResp.LedCadResp = NVRX_RECORDING_FAIL_FOR_ALL;
                buzLedResp.BuzCadResp = NVRX_RECORDING_FAIL_FOR_ALL;
                if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE,&buzLedResp) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
                }
                break;

            case SYS_APPL_INIT_RESP:
            case SYS_NET_CONN_RESP:
            case SYS_NET_DISCONN_RESP:
            default:
                break;
        }
    }

    close(gpioDrvFd);
	return (retVal);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   The Backup LED Status is passed as input parameter. According to the state passed Backup
 *          LedCadence is change.
 * @param   newBackUpLedState
 */
void SetBackupLedState(BACKUP_LED_STATE_e newBackUpLedState)
{
    if (newBackUpLedState >= BACKUP_LED_STATE_MAX)
	{
        return;
    }

    if (newBackUpLedState == NO_BACKUP_DEVICE_FOUND)
    {
        SetSystemStatusLed(SYS_NO_BKUP_RESP,ON);
    }
    else if (newBackUpLedState == BACKUP_DEVICE_NOT_ENOUGH_SPACE)
    {
        SetSystemStatusLed(SYS_BKUP_FULL_RESP,ON);
    }
    else
    {
        SetSystemStatusLed(SYS_NO_BKUP_RESP,OFF);
        SetSystemStatusLed(SYS_BKUP_FULL_RESP,OFF);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is use to set or clear alarm output for all the alarms. It takes alarm
 *          index and action as parameters and takes action according to configuration.
 * @param   alarmIndex
 * @param   action
 */
void ChangeAlarmStatus(UINT8 alarmIndex, BOOL action)
{
    BOOL                writeEvtF = FALSE;
    TIMER_INFO_t        timerInfo;
    ALARM_CONFIG_t      alarmCfg;
    LOG_EVENT_STATE_e   evtLog = EVENT_NORMAL;
    CHAR                detail[MAX_EVENT_DETAIL_SIZE];
    INT32               gpioDrvFd;
    UINT32              alarmAction;

    if ((alarmIndex >= OUTPUT_ALARM_MAX) || (alarmIndex >= GetNoOfAlarmOutput()))
    {
        return;
    }

    ReadSingleAlarmConfig(alarmIndex, &alarmCfg);
    alarmAction = (action == ON) ? ALMOUT_SET : ALMOUT_CLEAR;

    //current Alarm configuration is Active Interlock
    if(alarmCfg.activeMode == ALARM_INTERLOCK)
    {
        MUTEX_LOCK(alarmRunPara[alarmIndex].alarmAccessMutex);

        //if action is ON
        if(action == ON)
        {
            alarmRunPara[alarmIndex].runCnt++;
        }
        else
        {
            //if action is OFF
            if(alarmRunPara[alarmIndex].runCnt > 0)
            {
                alarmRunPara[alarmIndex].runCnt--;
            }
        }

        if(alarmCfg.alarmOutput == ENABLE)
        {
            if(action == ON)
            {
                if(alarmRunPara[alarmIndex].status == OFF)
                {
                    evtLog = EVENT_ACTIVE;
                    writeEvtF = TRUE;
                }
            }
            else
            {
                if ((alarmRunPara[alarmIndex].runCnt == 0) && (alarmRunPara[alarmIndex].status == ON))
                {
                    evtLog = EVENT_NORMAL;
                    writeEvtF = TRUE;
                }
            }

            if(writeEvtF == TRUE)
            {
                alarmRunPara[alarmIndex].status = action;

                gpioDrvFd = open(GPIO_DEVICE_NAME, READ_WRITE_MODE);
                if(gpioDrvFd < 0)
                {
                    EPRINT(INPUT_OUTPUT, "failed to open gpio driver: [err=%s]", STR_ERR);
                }
                else
                {
                    if(ioctl(gpioDrvFd, MXGPIO_SET_ALMOUT, &alarmAction) < 0)
                    {
                        EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_ALMOUT failed: [err=%s]", STR_ERR);
                    }

                    close(gpioDrvFd);
                }

                snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_INDEX_NO(alarmIndex));
                WriteEvent(LOG_ALARM_EVENT, LOG_ALARM_OUTPUT, detail, NULL, evtLog);
            }
        }

        DPRINT(INPUT_OUTPUT, "alarm status for interlock: [alarm=%d], [status=%d], [runCnt=%d]", alarmIndex, action, alarmRunPara[alarmIndex].runCnt);
        MUTEX_UNLOCK(alarmRunPara[alarmIndex].alarmAccessMutex);
    }
    else if ((alarmCfg.alarmOutput == ENABLE) && (action == ON))
    {
        timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(alarmCfg.pulsePeriod);
        timerInfo.data = alarmIndex;
        timerInfo.funcPtr = stopAlarm;

        if(alarmRunPara[alarmIndex].alarmTimerHandle == INVALID_TIMER_HANDLE)
        {
            writeEvtF = StartTimer(timerInfo, &alarmRunPara[alarmIndex].alarmTimerHandle);
            evtLog = EVENT_ACTIVE;
        }
        else
        {
            ReloadTimer(alarmRunPara[alarmIndex].alarmTimerHandle, timerInfo.count);
        }

        if(writeEvtF == SUCCESS)
        {
            MUTEX_LOCK(alarmRunPara[alarmIndex].alarmAccessMutex);
            alarmRunPara[alarmIndex].status = action;

            gpioDrvFd = open(GPIO_DEVICE_NAME, READ_WRITE_MODE);
            if(gpioDrvFd < 0)
            {
                EPRINT(INPUT_OUTPUT, "failed to open gpio driver: [err=%s]", STR_ERR);
            }
            else
            {
                if(ioctl(gpioDrvFd, MXGPIO_SET_ALMOUT, &alarmAction) < 0)
                {
                    EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_ALMOUT failed: [err=%s]", STR_ERR);
                }

                close(gpioDrvFd);
            }

            snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_INDEX_NO(alarmIndex));
            WriteEvent(LOG_ALARM_EVENT, LOG_ALARM_OUTPUT, detail, NULL, evtLog);
            MUTEX_UNLOCK(alarmRunPara[alarmIndex].alarmAccessMutex);
        }

        DPRINT(INPUT_OUTPUT, "alarm status for pulse: [alarm=%d], [status=%d]", alarmIndex, action);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to activate or in-activate the Alarm of input alarm index.
 * @param   alarmIndex
 * @param   action
 * @return
 */
NET_CMD_STATUS_e SetAlarmOut(UINT8 alarmIndex, BOOL action)
{
    ALARM_CONFIG_t  alarmCfg;
    CHAR            detail[MAX_EVENT_DETAIL_SIZE];
    UINT32          setAlarmAction;

    if ((alarmIndex >= OUTPUT_ALARM_MAX) || (alarmIndex >= GetNoOfAlarmOutput()))
 	{
        return CMD_PROCESS_ERROR;
    }

    /* Is alarm output enabled? */
    ReadSingleAlarmConfig(alarmIndex, &alarmCfg);
    if (alarmCfg.alarmOutput == DISABLE)
    {
        return CMD_ALARM_DISABLED;
    }

    MUTEX_LOCK(alarmRunPara[alarmIndex].alarmAccessMutex);
    alarmRunPara[alarmIndex].status = action;

    INT32 gpioDrvFd = open(GPIO_DEVICE_NAME, READ_WRITE_MODE);
    if(gpioDrvFd < 0)
    {
        EPRINT(INPUT_OUTPUT, "failed to open gpio driver: [err=%s]", STR_ERR);
    }
    else
    {
        setAlarmAction = (action == ON) ? ALMOUT_SET : ALMOUT_CLEAR;
        if(ioctl(gpioDrvFd, MXGPIO_SET_ALMOUT, &setAlarmAction) < 0)
        {
            EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_ALMOUT failed: [err=%s]", STR_ERR);
        }

        close(gpioDrvFd);
    }

    if (action == OFF)
    {
        DeleteTimer(&alarmRunPara[alarmIndex].alarmTimerHandle);
    }

    snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_INDEX_NO(alarmIndex) );
    WriteEvent(LOG_ALARM_EVENT, LOG_ALARM_OUTPUT, detail, NULL, action);
    MUTEX_UNLOCK(alarmRunPara[alarmIndex].alarmAccessMutex);
    DPRINT(INPUT_OUTPUT, "alarm out status: [alarm=%d], [status=%d]", alarmIndex, action);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns the status of the input alarm whether it is in active or inactive state.
 * @param   alarmIndex
 * @param   dummy
 * @return
 */
BOOL GetAlarmStatus(UINT8 alarmIndex, UINT8 dummy)
{
    if ((alarmIndex >= OUTPUT_ALARM_MAX) || (alarmIndex >= GetNoOfAlarmOutput()))
    {
        return OFF;
    }

    MUTEX_LOCK(alarmRunPara[alarmIndex].alarmAccessMutex);
    BOOL alarmStatus = (BOOL)alarmRunPara[alarmIndex].status;
    MUTEX_UNLOCK(alarmRunPara[alarmIndex].alarmAccessMutex);
    return alarmStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns the status of the sensor input whether it is in active or inactive state.
 * @param   sensorIndex
 * @param   dummy
 * @return
 */
BOOL GetSensorStatus(UINT8 sensorIndex, UINT8 dummy)
{
    if ((sensorIndex >= INPUT_SENSOR_MAX) || (sensorIndex >= GetNoOfSensorInput()))
    {
        return INACTIVE;
    }

    MUTEX_LOCK(ioAccessMutex[sensorIndex]);
    BOOL sensorState = currSensorState[sensorIndex];
    MUTEX_UNLOCK(ioAccessMutex[sensorIndex]);
    return sensorState;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Call back function to stop alarm outupt.
 * @param   alarmIndex
 */
static void stopAlarm(UINT32 alarmIndex)
{
 	CHAR	detail[MAX_EVENT_DETAIL_SIZE];
 	UINT32	stopAlarm;

    if ((alarmIndex >= OUTPUT_ALARM_MAX) || (alarmIndex >= GetNoOfAlarmOutput()))
    {
        return;
    }

    MUTEX_LOCK(alarmRunPara[alarmIndex].alarmAccessMutex);
    alarmRunPara[alarmIndex].status = OFF;
    stopAlarm = ALMOUT_CLEAR;

    INT32 gpioDrvFd = open(GPIO_DEVICE_NAME, READ_WRITE_MODE);
    if(gpioDrvFd < 0)
    {
        EPRINT(INPUT_OUTPUT, "failed to open gpio driver: [err=%s]", STR_ERR);
    }
    else
    {
        if(ioctl(gpioDrvFd, MXGPIO_SET_ALMOUT, &stopAlarm) < 0)
        {
            EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_ALMOUT failed: [err=%s]", STR_ERR);
        }

        close(gpioDrvFd);
    }

    snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_INDEX_NO(alarmIndex));
    WriteEvent(LOG_ALARM_EVENT, LOG_ALARM_OUTPUT, detail, NULL, EVENT_NORMAL);
    MUTEX_UNLOCK(alarmRunPara[alarmIndex].alarmAccessMutex);
    DeleteTimer(&alarmRunPara[alarmIndex].alarmTimerHandle);
    DPRINT(INPUT_OUTPUT, "stop alarm: [alarm=%d]", alarmIndex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function do system buzzer state in off
 * @return
 */
BOOL MuteBuzzer(void)
{
    BUZZER_LED_RESP_t buzLedResp;

    INT32 gpioDrvFd = open(GPIO_DEVICE_NAME, READ_WRITE_MODE);
    if(gpioDrvFd < 0)
    {
        EPRINT(INPUT_OUTPUT, "failed to open gpio driver: [err=%s]", STR_ERR);
        return FAIL;
    }

    buzLedResp.LedCadResp = NVRX_RESP_NONE;
    buzLedResp.BuzCadResp = NVRX_RESP_ALL;

    if(ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE, &buzLedResp) < 0)
    {
        EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_CLR_CADENCE failed: [err=%s]", STR_ERR);
        close(gpioDrvFd);
        return FAIL;
    }

    close(gpioDrvFd);

    MUTEX_LOCK(buzzerStatusMutex);
    buzzerStatus = OFF;
    MUTEX_UNLOCK(buzzerStatusMutex);

    /* send event to gui for buzzer status normal */
    WriteEvent(LOG_OTHER_EVENT, LOG_BUZZER_STATUS, NULL, NULL, EVENT_NORMAL);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function return current buzzer status
 * @param   dummy1
 * @param   dummy2
 * @return
 */
BOOL GetAllBuzzerStatus(UINT8 dummy1, UINT8 dummy2)
{
    UINT32 value = OFF;

    INT32 gpioDrvFd = open(GPIO_DEVICE_NAME, READ_WRITE_MODE);
    if(gpioDrvFd < 0)
    {
        EPRINT(INPUT_OUTPUT, "failed to open gpio driver: [err=%s]", STR_ERR);
        return OFF;
    }

    if(ioctl(gpioDrvFd, MXGPIO_GET_BUZ_STS, &value) < 0)
    {
        EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_GET_BUZ_STS failed: [err=%s]", STR_ERR);
        close(gpioDrvFd);
        return OFF;
    }

    close(gpioDrvFd);
    return (value == OFF) ? OFF : ON;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is a callback function of the timer within which we turn on and turn off
 *          the leds and buzzer every 1 second for 2cycles. Then start the timer checkIOstatus.
 * @param   arg
 */
static void initLedBuzTimerCB(UINT32 arg)
{
    static UINT8    initLedBuzTmrCnt = 0;
	TIMER_INFO_t	checkIOStatusTimer;

    if (initLedBuzTmrCnt == 0)
	{
        UINT32 value = NVRX_APPL_INIT_RESP;
        INT32 gpioDrvFd = open(GPIO_DEVICE_NAME, READ_WRITE_MODE);
        if (gpioDrvFd < 0)
        {
            EPRINT(INPUT_OUTPUT, "failed to open gpio driver: [err=%s]", STR_ERR);
        }
        else
        {
            if (ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
            {
                EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", STR_ERR);
            }

            close(gpioDrvFd);
        }
	}

    initLedBuzTmrCnt++;
    if (initLedBuzTmrCnt < INIT_LED_BUZZER_CYCLE)
	{
        return;
    }

    // Remove init timer
    DeleteTimer(&initLedBuzTimerHandle);

    //start the checkIostatus timer
    checkIOStatusTimer.count = CONVERT_MSEC_TO_TIMER_COUNT(100);
    checkIOStatusTimer.data = 0;
    checkIOStatusTimer.funcPtr = checkIOStatus;
    StartTimer(checkIOStatusTimer, &checkIOStatusTimerHandle);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is a callback function of the timer within which we check the input received
 *          from the sensors, backup and reset button. It also processes output pins.
 * @param   arg
 */
static void checkIOStatus(UINT32 arg)
{
    INPUT_PIN_e     inputNo;
    BOOL            inputStatus;
    INPUT_STATUS_t	allInputStatus = {{0}};

    /* Open GPIO driver for IO status */
    INT32 gpioDrvFd = open(GPIO_DEVICE_NAME, READ_ONLY_MODE);
    if (gpioDrvFd == INVALID_FILE_FD)
    {
        EPRINT(INPUT_OUTPUT, "fail to open gpio driver for input status: [err=%s]", STR_ERR);
        return;
    }

    /* Get all IO status */
    if (ioctl(gpioDrvFd, MXGPIO_GET_ALL_INPUT, &allInputStatus) < 0)
    {
        EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_GET_ALL_INPUT failed: [err=%s]", STR_ERR);
        close(gpioDrvFd);
        return;
    }

    /* Close GPIO driver */
    close(gpioDrvFd);

    /* Map IO status from driver to application */
    for (inputNo = 0; inputNo < INPUT_MAX; inputNo++)
    {
        inputStatus = INPUT_INACTIVE;
        switch(inputNo)
        {
            case INPUT_SENSOR_1:
                inputStatus = allInputStatus.InputPinStatus[ALMIN1_INPUT];
                break;

            case INPUT_SENSOR_2:
                inputStatus = allInputStatus.InputPinStatus[ALMIN2_INPUT];
                break;

            case INPUT_RESET_BUTTON:
                inputStatus = allInputStatus.InputPinStatus[IPDEF_INPUT];
                break;

            default:
                break;
        }

        /* Is input status valid? */
        if ((inputStatus != INPUT_INACTIVE) && (inputStatus != INPUT_ACTIVE))
        {
            continue;
        }

        /* Convert IO status to application format */
        inputStatus = (inputStatus == INPUT_ACTIVE) ? TRUE : FALSE;

        /* If status is same then nothing to do */
        if (inputStatus == inputStatusMap[inputNo].inputPinStatus)
        {
            inputStatusMap[inputNo].statusHoldCnt = 0;
            continue;
        }

        /* If status is changed then wait for atleast 400ms to declare it */
        inputStatusMap[inputNo].statusHoldCnt++;
        if (inputStatusMap[inputNo].statusHoldCnt >= INPUT_PIN_STATUS_CNT)
        {
            /* Set new status and take action against it */
            inputStatusMap[inputNo].statusHoldCnt = 0;
            inputStatusMap[inputNo].inputPinStatus = inputStatus;
            dispatchIOStatus(inputNo, inputStatusMap[inputNo].inputPinStatus);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks the input pin status and accordingly takes the action.
 * @param   inputNo
 * @param   status
 */
static void dispatchIOStatus(INPUT_PIN_e inputNo, BOOL status)
{
    SENSOR_CONFIG_t sensorConfig;
    CHAR            detail[MAX_EVENT_DETAIL_SIZE];
    TIMER_INFO_t    timerInfo;

	switch(inputNo)
	{
        default:
        {
            if ((inputNo >= INPUT_SENSOR_MAX) || (inputNo >= GetNoOfSensorInput()))
            {
                break;
            }

            ReadSingleSensorConfig(inputNo, &sensorConfig);
            if (sensorConfig.sensorDetect == DISABLE)
            {
                break;
            }

            if (((sensorConfig.normalMode == NORMALLY_OPEN) && (status == FALSE)) || ((sensorConfig.normalMode == NORMALLY_CLOSE) && (status == TRUE)))
            {
                if(sensorConfig.debounceTime > 0)
                {
                    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT (sensorConfig.debounceTime);
                    timerInfo.data = inputNo;
                    timerInfo.funcPtr = changeSensorInputCB;

                    if(sensorInputTimerHandle[inputNo] == INVALID_TIMER_HANDLE)
                    {
                        StartTimer(timerInfo, &sensorInputTimerHandle[inputNo]);
                    }
                    else
                    {
                        ReloadTimer(sensorInputTimerHandle[inputNo], timerInfo.count);
                    }
                }
                else
                {
                    MUTEX_LOCK(ioAccessMutex[inputNo]);
                    if(currSensorState[inputNo] == INACTIVE)
                    {
                        currSensorState[inputNo] = ACTIVE;
                        snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_INDEX_NO(inputNo) );
                        WriteEvent(LOG_SENSOR_EVENT, LOG_SENSOR_INPUT, detail, sensorConfig.name, EVENT_ACTIVE);
                        SensorEventNotify(inputNo, ACTIVE);
                    }
                    MUTEX_UNLOCK(ioAccessMutex[inputNo]);
                }
            }
            else
            {
                MUTEX_LOCK(ioAccessMutex[inputNo]);
                if(currSensorState[inputNo] == ACTIVE)
                {
                    currSensorState[inputNo] = INACTIVE;
                    snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_INDEX_NO(inputNo) );
                    WriteEvent(LOG_SENSOR_EVENT, LOG_SENSOR_INPUT, detail, sensorConfig.name, EVENT_NORMAL);
                    SensorEventNotify(inputNo, INACTIVE);
                }
                MUTEX_UNLOCK(ioAccessMutex[inputNo]);
                DeleteTimer(&sensorInputTimerHandle[inputNo]);
            }
        }
        break;

        case INPUT_RESET_BUTTON:
        {
            if(status == OFF)
            {
                DPRINT(SYS_LOG, "reset switch pressed");
                if (resetBtnTimerHandle == INVALID_TIMER_HANDLE)
                {
                    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(RESET_BTN_TIME);
                    timerInfo.data = 0;
                    timerInfo.funcPtr = resetBtnTimerCB;
                    StartTimer(timerInfo, &resetBtnTimerHandle);
                }
            }
            else
            {
                DPRINT(SYS_LOG, "reset switch release");
                DeleteTimer(&resetBtnTimerHandle);
            }
        }
        break;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is call back function of input change and its sends  notification to evant handler.
 * @param   arg
 */
static void resetBtnTimerCB(UINT32 arg)
{
	WriteEvent(LOG_SYSTEM_EVENT, LOG_SYSTEM_RESET, NULL, NULL, EVENT_ALERT);
	ResetButtonAction();
	PrepForPowerAction(REBOOT_DEVICE, EVENT_AUTO, "Reset Button");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is call back function of input change and its sends notification to evant handler.
 * @param   sensorIndex
 */
static void changeSensorInputCB(UINT32 sensorIndex)
{
    SENSOR_CONFIG_t sensorConfig;
    CHAR            detail[MAX_EVENT_DETAIL_SIZE];

    ReadSingleSensorConfig(sensorIndex, &sensorConfig);
    MUTEX_LOCK(ioAccessMutex[sensorIndex]);
    if(currSensorState[sensorIndex] == INACTIVE)
	{
        currSensorState[sensorIndex] = ACTIVE;
        snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_INDEX_NO(sensorIndex));
        WriteEvent(LOG_SENSOR_EVENT, LOG_SENSOR_INPUT, detail, sensorConfig.name, EVENT_ACTIVE);
        SensorEventNotify(sensorIndex, ACTIVE);
	}
    MUTEX_UNLOCK(ioAccessMutex[sensorIndex]);
    DeleteTimer(&sensorInputTimerHandle[sensorIndex]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function on/off usb control pin
 * @param   usbPower
 * @return
 */
BOOL UsbControlChangeCmd(UINT32 usbPower)
{
    if (usbPower >= USBPOWER_LAST)
    {
        return FAIL;
    }

    INT32 gpioDrvFd = open(GPIO_DEVICE_NAME, READ_WRITE_MODE);
    if(gpioDrvFd < 0)
    {
        EPRINT(INPUT_OUTPUT, "failed to open gpio driver: [err=%s]", STR_ERR);
        return FAIL;
	}

    if(ioctl(gpioDrvFd, MXGPIO_SET_USBPOWER, &usbPower) < 0)
    {
        EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_USBPOWER failed: [usbPower=%d], [err=%s]", usbPower, STR_ERR);
        close(gpioDrvFd);
        return FAIL;
    }

	close(gpioDrvFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives the current temperature of the cpu.
 * @return  Temperature in celsius
 */
UINT32 GetCpuTemperature(void)
{
    UINT32	cpuTempCels = 0;

    #if defined(RK3568_NVRL) || defined(RK3588_NVRH)
    CHAR    cpuTempStr[8];
    INT32   readCnt;

    INT32 gpioDrvFd = open(CPU_TEMPERATURE_FILE, READ_ONLY_MODE);
    if(gpioDrvFd < 0)
    {
        EPRINT(INPUT_OUTPUT, "failed to open cpu temperature file: [err=%s]", STR_ERR);
        return cpuTempCels;
    }

    /* Get temperature in string format. It provides temperature in celsius with 1000 multiplication */
    readCnt = read(gpioDrvFd, &cpuTempStr, sizeof(cpuTempStr)-1);
    if (readCnt < 0)
    {
        EPRINT(INPUT_OUTPUT, "failed to read data from cpu temperature file: [err=%s]", STR_ERR);
    }
    else
    {
        cpuTempStr[readCnt] = '\0';
        cpuTempCels = (atoi(cpuTempStr) / 1000);
    }
    #else
    INT32 gpioDrvFd = open(GPIO_DEVICE_NAME, READ_WRITE_MODE);
    if(gpioDrvFd < 0)
    {
        EPRINT(INPUT_OUTPUT, "failed to open gpio driver: [err=%s]", STR_ERR);
        return cpuTempCels;
    }

    if(ioctl(gpioDrvFd, MXGPIO_GET_CORETEMPE, &cpuTempCels) < 0)
    {
        EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_GET_CORETEMPE failed: [err=%s]", STR_ERR);
    }
    #endif

    close(gpioDrvFd);
    return cpuTempCels;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Function will decide board type and version of board from input gpio value.
 * @return
 */
NVR_VARIANT_e GetProductVariant(void)
{
    UINT32          boardType = 0;
    UINT32          boardHwVersion = 0;
    NVR_VARIANT_e   productVariant = NVR_VARIANT_MAX;

    memset(&boardInfo, 0, sizeof(boardInfo));
    INT32 gpioDrvFd = open(GPIO_DEVICE_NAME, READ_WRITE_MODE);
    if(gpioDrvFd < 0)
    {
        EPRINT(INPUT_OUTPUT, "failed to open gpio driver: [err=%s]", STR_ERR);
        return productVariant;
    }

    if (ioctl(gpioDrvFd, MXGPIO_GET_CARD_TYPE, &boardType) < 0)
    {
        EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_GET_CARD_TYPE failed: [err=%s]", STR_ERR);
        close(gpioDrvFd);
        return productVariant;
    }

    if(ioctl(gpioDrvFd, MXGPIO_GET_BOARD_VERSION, &boardHwVersion) < 0)
    {
        EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_GET_BOARD_VERSION failed: [err=%s]", STR_ERR);
        close(gpioDrvFd);
        return productVariant;
    }

    close(gpioDrvFd);
    EPRINT(INPUT_OUTPUT, "device details: [BoardType=%d], [BoardHwVersion=%d]", boardType, boardHwVersion);

    switch(boardType)
    {
        #if defined(RK3568_NVRL)
        case NVR_MODEL_0401XS:
            productVariant = NVR0401XSP2;
            break;

        case NVR_MODEL_0801X:
            productVariant = NVR0801XP2;
            break;

        case NVR_MODEL_1601X:
            productVariant = NVR1601XP2;
            break;

        case NVR_MODEL_1602X:
            productVariant = NVR1602XP2;
            break;

        case NVR_MODEL_0801XS:
            productVariant = NVR0801XSP2;
            break;

        case NVR_MODEL_1601XS:
            productVariant = NVR1601XSP2;
            break;

        default:
            /* When invalid board type detected, set lowest variant */
            EPRINT(INPUT_OUTPUT, "invld board type detected, set to default: [boardType=%d]", boardType);
            productVariant = NVR0801XP2;
            break;

        #elif defined(RK3588_NVRH)
        #if defined(OEM_JCI)
        case NVR_MODEL_HRIN_1208:
            productVariant = HRIN_1208_18_SR;
            break;

        case NVR_MODEL_HRIN_2808:
            productVariant = HRIN_2808_18_SR;
            break;

        case NVR_MODEL_HRIN_4808:
            productVariant = HRIN_4808_18_SR;
            break;

        case NVR_MODEL_HRIN_6408:
            productVariant = HRIN_6408_18_SR;
            break;

        default:
            /* When invalid board type detected, set lowest variant */
            EPRINT(INPUT_OUTPUT, "invld board type detected, set to default: [boardType=%d]", boardType);
            productVariant = HRIN_1208_18_SR;
            break;
        #else
        case NVR_MODEL_3202X:
            productVariant = NVR3202XP2;
            break;

        case NVR_MODEL_3204X:
            productVariant = NVR3204XP2;
            break;

        case NVR_MODEL_6404X:
            productVariant = NVR6404XP2;
            break;

        case NVR_MODEL_6408X:
            productVariant = NVR6408XP2;
            break;

        case NVR_MODEL_9608X:
            productVariant = NVR9608XP2;
            break;

        default:
            /* When invalid board type detected, set lowest variant */
            EPRINT(INPUT_OUTPUT, "invld board type detected, set to default: [boardType=%d]", boardType);
            productVariant = NVR3202XP2;
            break;
        #endif
        #else
        case NVR_MODEL_0801X:
            productVariant = NVR0801X;
            break;

        case NVR_MODEL_1601X:
            productVariant = NVR1601X;
            break;

        case NVR_MODEL_1602X:
            productVariant = NVR1602X;
            break;

        case NVR_MODEL_3202X:
            productVariant = NVR3202X;
            break;

        case NVR_MODEL_3204X:
            productVariant = NVR3204X;
            break;

        case NVR_MODEL_6404X:
            productVariant = NVR6404X;
            break;

        case NVR_MODEL_6408X:
            productVariant = NVR6408X;
            break;

        default:
            /* When invalid board type detected, set lowest variant */
            EPRINT(INPUT_OUTPUT, "invld board type detected, set to default: [boardType=%d]", boardType);
            productVariant = NVR0801X;
            break;
        #endif
    }

    /* Set derived values */
    boardInfo.isExtRtcAvail = YES;
    boardInfo.boardVersion = boardHwVersion;

    #if defined(RK3568_NVRL)
    boardInfo.isMultiplierAvail = NO;
    if (boardHwVersion > NVR_XP2_BOARD_V1R1)
    {
        EPRINT(INPUT_OUTPUT, "invld board hardware version, setting to default: [boardHwVersion=%d]", boardHwVersion);
        boardInfo.boardVersion = NVR_XP2_BOARD_V1R0;
    }
    #elif defined(RK3588_NVRH)
    #if defined(OEM_JCI)
    boardInfo.isMultiplierAvail = YES;
    #else
    /* We have no SATA multiplier for NVR_MODEL_3202X */
    boardInfo.isMultiplierAvail = (productVariant == NVR3202XP2) ? NO : YES;
    #endif
    if (boardHwVersion > NVR_XP2_BOARD_V1R0)
    {
        EPRINT(INPUT_OUTPUT, "invld board hardware version, setting to default: [boardHwVersion=%d]", boardHwVersion);
        boardInfo.boardVersion = NVR_XP2_BOARD_V1R0;
    }
    #else
    switch(boardHwVersion)
    {
        case NVR_X_BOARD_V1R2 :
            boardInfo.isMultiplierAvail = YES;
            break;

        case NVR_X_BOARD_V1R3:
            boardInfo.isExtRtcAvail = NO;
            boardInfo.isMultiplierAvail = YES;
            break;

        case NVR_X_BOARD_V2R2:
            boardInfo.isExtRtcAvail = NO;
            boardInfo.isMultiplierAvail = (boardType == NVR_MODEL_6408X) ? YES : NO;
            break;

        default:
            EPRINT(INPUT_OUTPUT, "invld board hardware version, setting to default: [boardHwVersion=%d]", boardHwVersion);
            boardInfo.boardVersion = NVR_X_BOARD_V1R2;
            boardInfo.isMultiplierAvail = YES;
            break;
    }
    #endif

    return productVariant;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get NVR board hardware version string
 * @return  Version String
 */
const CHAR *GetHardwareBoardVersionStr(void)
{
    switch(boardInfo.boardVersion)
    {
        #if defined(RK3568_NVRL) || defined(RK3588_NVRH)
        case NVR_XP2_BOARD_V1R0:
            return "V1R0";

        case NVR_XP2_BOARD_V1R1:
            return "V1R1";

        #else
        case NVR_X_BOARD_V1R2:
            return "V1R2";

        case NVR_X_BOARD_V1R3:
            return "V1R3";

        case NVR_X_BOARD_V2R2:
            return "V2R2";
        #endif

        default:
            return "UNKNOWN";
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Is external RTC available on the board?
 * @return  Returns TRUE if available else returns FALSE
 */
BOOL IsExternalRtcAvailable(void)
{
    return boardInfo.isExtRtcAvail;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Is disk multiplier available on the board?
 * @return  Returns TRUE if available else returns FALSE
 */
BOOL IsDiskMultiplierAvailable(void)
{
    return boardInfo.isMultiplierAvail;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Function will start and stop buzzer for CPU temperature monitoring.
 * @param   status
 */
void StartStopCpuTempBuzz(BOOL status)
{
    UINT32              value = NVRX_CPUTEMP_HIGH;
    BUZZER_LED_RESP_t 	buzLedResp;

    INT32 gpioDrvFd = open(GPIO_DEVICE_NAME, READ_WRITE_MODE);
    if (gpioDrvFd < 0)
    {
        EPRINT(INPUT_OUTPUT, "failed to open gpio driver: [err=%s]", STR_ERR);
        return;
    }

    if (status == START)
    {
        if (ioctl(gpioDrvFd, MXGPIO_SET_CADENCE, &value) < 0)
        {
            EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed [%s]", STR_ERR);
        }
    }
    else
    {
        buzLedResp.LedCadResp = NVRX_CPUTEMP_HIGH;
        buzLedResp.BuzCadResp = NVRX_CPUTEMP_HIGH;
        if (ioctl(gpioDrvFd, MXGPIO_CLR_CADENCE, &buzLedResp) < 0)
        {
            EPRINT(INPUT_OUTPUT, "ioctl MXGPIO_SET_CADENCE failed [%s]", STR_ERR);
        }
    }

    /* Close GPIO driver */
    close(gpioDrvFd);
}

//-------------------------------------------------------------------------------------
/**
 * @brief   setBuzzerStatus
 * @param   status
 */
void setBuzzerStatus(BOOL status)
{
    MUTEX_LOCK(buzzerStatusMutex);
    if (status == ON)
    {
        /* if buzzer is off change to on */
        if(buzzerStatus == OFF)
        {
            WriteEvent(LOG_OTHER_EVENT, LOG_BUZZER_STATUS, NULL, NULL, EVENT_ACTIVE);
            buzzerStatus = ON;
        }
    }
    else
    {
        /* if buzzer is on change to off */
        if(buzzerStatus == ON)
        {
            WriteEvent(LOG_OTHER_EVENT, LOG_BUZZER_STATUS, NULL, NULL, EVENT_NORMAL);
            buzzerStatus = OFF;
        }
    }
    MUTEX_UNLOCK(buzzerStatusMutex);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
