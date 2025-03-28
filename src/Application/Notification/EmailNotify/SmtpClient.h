#if !defined MxSmtpApi_H
#define MxSmtpApi_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		SmtpClient.h
@brief      File containing the declaration of different functions for sending emails.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "ConfigApi.h"

//#################################################################################################
// @PROTOTYPE
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL InitSmtpClient(void);
//-------------------------------------------------------------------------------------------------
BOOL DeinitSmtpClient(void);
//-------------------------------------------------------------------------------------------------
void UpdateSmtpConfig(SMTP_CONFIG_t newCopy, SMTP_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ProcessEmail(EMAIL_PARAMETER_t *pEmailParam, CHARPTR pAttachment);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e SendTestEmail(CHARPTR pToField, SMTP_CONFIG_t *pSmptConfig, NW_CMD_REPLY_CB callback, INT32 clientFd);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e SendOtpEmail(SMTP_CONFIG_t *pSmptConfig, EMAIL_PARAMETER_t *pEmailParam, NW_CMD_REPLY_CB callback, INT32 clientFd);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif
