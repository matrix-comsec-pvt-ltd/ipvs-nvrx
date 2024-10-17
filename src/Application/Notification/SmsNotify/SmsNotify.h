#if !defined SMSNOTIFY_H
#define SMSNOTIFY_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		SmsNotify.h
@brief      This module Provides APIs to Send / Test SMS Service. It uses "Queue Module" to manage
            SMS Queue. It reads SMS Configuration  using API of Configuration Module and based on
            "MODE" ( HTTP / BROADBAND) selected by User, It gives particular Task to SMSonHttp Module.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Config.h"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitSmsNotification(void);
//-------------------------------------------------------------------------------------------------
BOOL SendSmsNotification(SMS_PARAMTER_t *smsParam);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e TestSmsNotification(SMS_CONFIG_t * smsDataPtr, CHARPTR deviceNumber, NW_CMD_REPLY_CB callBack, INT32 connFd);
//-------------------------------------------------------------------------------------------------
void DeInitSmsNotification(void);
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // SMSNOTIFY_H
