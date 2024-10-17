#if !defined DOORCOMMAND_H
#define DOORCOMMAND_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DoorCommand.h
@brief      This file contains functions to process the commands for Cosec integration
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "MxTypedef.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_COSEC_USERID_LEN		16
#define MAX_COSEC_USERNAME_LEN 		16
#define MAX_COSEC_DOORNAME_LEN   	32
#define MAX_COSEC_EMAIL_LEN      	101

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    COSEC_DOOR_EVENT_USER_ALLOWED = 0,
    COSEC_DOOR_EVENT_USER_DENIED,
    COSEC_DOOR_EVENT_USER_NOT_IDENTIFIED,
    COSEC_DOOR_EVENT_AUX_IN1_STATUS_CHANGED,
    COSEC_DOOR_EVENT_DURESS_DETECTED,
    COSEC_DOOR_EVENT_DEAD_MAN_TIMER_EXPIRED,
    COSEC_DOOR_EVENT_PANIC_ALARM,
    COSEC_DOOR_EVENT_DOOR_ABNORMAL,
    COSEC_DOOR_EVENT_DOOR_FORCE_OPENED,
    COSEC_DOOR_EVENT_TAMPER_ALARM,
    COSEC_DOOR_EVENT_INTERCOM_PANIC,
    COSEC_DOOR_EVENT_AUX_IN2_STATUS_CHANGED,
    COSEC_DOOR_EVENT_AUX_IN3_STATUS_CHANGED,
    COSEC_DOOR_EVENT_AUX_IN4_STATUS_CHANGED,
    COSEC_DOOR_EVENT_AUX_IN5_STATUS_CHANGED,
    COSEC_DOOR_EVENT_AUX_IN6_STATUS_CHANGED,
    COSEC_DOOR_EVENT_AUX_IN7_STATUS_CHANGED,
    COSEC_DOOR_EVENT_AUX_IN8_STATUS_CHANGED,
    COSEC_DOOR_EVENT_MAX

}COSEC_DOOR_EVENT_e;

typedef	enum
{
	COSEC_REC,
	COSEC_IMAGE_UPLD,
	COSEC_EMAIL_NOTF,
	COSEC_POPUP,
	COSEC_PRESET_POS,
	MAX_COSEC_CMD
}COSEC_CMD_NAME_e;

typedef struct
{
	BOOL						enable;
	UINT32						doorType;
	UINT32 						doorMid;
	UINT32 						doorDid;
	CHAR						doorName[MAX_COSEC_DOORNAME_LEN];
	COSEC_CMD_NAME_e			doorCommand;
    CAMERA_BIT_MASK_t           cameraMask;
	UINT32						doorCmdData;
	CHAR						userId[MAX_COSEC_USERID_LEN];
	CHAR						userName[MAX_COSEC_USERNAME_LEN];
    COSEC_DOOR_EVENT_e          eventCode;
	CHAR						emailId[MAX_COSEC_EMAIL_LEN];
	UINT32						dateTimeSec;
}COSEC_INTEGRATION_t;

//#################################################################################################
// @EXTERN VARIABLES
//#################################################################################################
extern COSEC_INTEGRATION_t      cosecInfoVideoPopup;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL ProcessDoorCommand(CHARPTR commandStrPtr);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  //DOORCOMMAND_H
