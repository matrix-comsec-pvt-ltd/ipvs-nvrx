#ifndef INPUTOUTPUT_H_
#define INPUTOUTPUT_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		InputOutput.h
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
/* Application Includes */
#include "Config.h"

/* Driver Includes */
#include "MxGpioDrv.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
/* Priority wise lowest to highest states of system status led */
typedef enum
{
	SYS_LED_IDLE_STATE = 0,
	SYS_APPL_INIT_RESP,
	SYS_NET_CONN_RESP,
	SYS_NET_DISCONN_RESP,
	SYS_INCOMPLETE_VOLUME,
	SYS_STORAGE_ALERT,
	SYS_FULL_VOL,
	SYS_FORMATTING,
	SYS_FIRMWARE_UPGRADE_INPROCESS,
	SYS_UPGRADE_FAILED,
	SYS_TRG_EVT_RESP,
	SYS_BKUP_FULL_RESP,
	SYS_NO_BKUP_RESP,
	SYS_NORMAL_STATE,
	SYS_RECORDING_FAIL,
	SYS_RECORDING_FAIL_FOR_ALL,
	SYS_LED_STATES_MAX
}SYSTEM_LED_STATE_e;

/* Available backup states */
typedef enum
{
	BACKUP_NOT_GOING_ON = 0,
	BACKUP_NO_ACITIVITY,
	BACKUP_GOING_ON,
	BACKUP_DEVICE_NOT_ENOUGH_SPACE,
	NO_BACKUP_DEVICE_FOUND,
	FORMATTING_BACKUP_DEVICE,
	BACKUP_LED_STATE_MAX
}BACKUP_LED_STATE_e;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitIO(void);
//-------------------------------------------------------------------------------------------------
void DeinitIO(void);
//-------------------------------------------------------------------------------------------------
void UpdateSensorConfig(SENSOR_CONFIG_t newSensorConfig, SENSOR_CONFIG_t *oldSensorConfig, UINT8 sensorNo);
//-------------------------------------------------------------------------------------------------
void UpdateAlarmConfig(ALARM_CONFIG_t newCfg, ALARM_CONFIG_t *oldCfg, UINT8 alarmNo);
//-------------------------------------------------------------------------------------------------
BOOL SetSystemStatusLed(SYSTEM_LED_STATE_e newSystemLedState, BOOL action);
//-------------------------------------------------------------------------------------------------
void SetBackupLedState(BACKUP_LED_STATE_e newBackUpLedState);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e SetAlarmOut(UINT8 alarmIndex, BOOL action);
//-------------------------------------------------------------------------------------------------
BOOL GetAlarmStatus(UINT8 alarmIndex, UINT8	dummy);
//-------------------------------------------------------------------------------------------------
BOOL GetSensorStatus(UINT8 sensorIndex, UINT8 dummy);
//-------------------------------------------------------------------------------------------------
void ChangeAlarmStatus(UINT8 alarmIndex, BOOL action);
//-------------------------------------------------------------------------------------------------
BOOL MuteBuzzer(void);
//-------------------------------------------------------------------------------------------------
BOOL GetAllBuzzerStatus(UINT8 dummy1, UINT8 dummy2);
//-------------------------------------------------------------------------------------------------
UINT32 GetCpuTemperature(void);
//-------------------------------------------------------------------------------------------------
BOOL UsbControlChangeCmd(UINT32 usbPower);
//-------------------------------------------------------------------------------------------------
NVR_VARIANT_e GetProductVariant(void);
//-------------------------------------------------------------------------------------------------
void StartStopCpuTempBuzz(BOOL status);
//-------------------------------------------------------------------------------------------------
const CHAR *GetHardwareBoardVersionStr(void);
//-------------------------------------------------------------------------------------------------
BOOL IsExternalRtcAvailable(void);
//-------------------------------------------------------------------------------------------------
BOOL IsDiskMultiplierAvailable(void);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* INPUTOUTPUT_H_ */
