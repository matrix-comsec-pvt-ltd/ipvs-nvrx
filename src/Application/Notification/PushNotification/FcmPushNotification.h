#if !defined FCM_PUSH_NOTIFICATION_H
#define FCM_PUSH_NOTIFICATION_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		FcmPushNotification.h
@brief      This module Provides APIs to Send Push Notifications to Android/iOS devices
            using FCM service by Google.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Config.h"

//#################################################################################################
// @DEFINES
//#################################################################################################

/* length of individual payload field */
#define FCM_PAYLOAD_ELEMENT_LEN_MAX         (128)

//#################################################################################################
// @TYPEDEFS
//#################################################################################################

/**
 * @brief push notification param given by the parent module to send notification
 */
typedef struct
{
    CHAR title[FCM_PAYLOAD_ELEMENT_LEN_MAX];
    CHAR body[FCM_PAYLOAD_ELEMENT_LEN_MAX];
    CHAR dateTime[FCM_PAYLOAD_ELEMENT_LEN_MAX];

} PUSH_NOTIFICATION_EVENT_PARAM_t;

/**
 * @brief push notification device list
 */
typedef struct
{
    CHAR username[MAX_USER_ACCOUNT_USERNAME_WIDTH];
    CHAR deviceFcmToken[FCM_TOKEN_LENGTH_MAX];
    CHAR deviceModelName[FCM_DEVICE_MODEL_NAME_LEN_MAX];
    UINT32 deviceInactivityTimer;

} PUSH_NOTIFY_DEVICE_STS_LIST_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL InitFcmPushNotification(void);
//-------------------------------------------------------------------------------------------------
BOOL DeinitFcmPushNotification(void);
//-------------------------------------------------------------------------------------------------
void FcmPushNotifyMobileUserLogin(UINT8 sessionIndex, CHARPTR username, CHARPTR fcmToken);
//-------------------------------------------------------------------------------------------------
void FcmPushNotifyMobileUserLogout(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
void FcmPushDeleteSystemUserNotify(CHARPTR username);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ManagePushNotificationSts(UINT8 sessionIndex, BOOL enable, CHARPTR fcmToken, CHARPTR deviceModelname);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetDevicePushNotificationEnableSts(CHARPTR fcmToken);
//-------------------------------------------------------------------------------------------------
BOOL FcmPushNotifyConfigUpdate(FCM_PUSH_NOTIFY_CONFIG_t newCopyPtr, FCM_PUSH_NOTIFY_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ProcessFcmPushNotification(PUSH_NOTIFICATION_EVENT_PARAM_t *pEventParam);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetPushNotificationDeviceStsList(PUSH_NOTIFY_DEVICE_STS_LIST_t *pDeviceList);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e DeletePushNotificationDevice(CHARPTR fcmToken);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // FCM_PUSH_NOTIFICATION_H

// ----------------------------------------------------------------
// Response of Access token request when JWT claims exp is more than 1 hour
// ----------------------------------------------------------------

/*
HTTP response code: 400 Bad Request

{
    "error":"invalid_grant",
    "error_description":"Invalid JWT: Token must be a short-lived token (60 minutes) and in a reasonable timeframe.
                         Check your iat and exp values in the JWT claim."
}
*/

// ----------------------------------------------------------------
// Response of Push Notification when oauth2.0 token is missing
// ----------------------------------------------------------------

/*
HTTP response code: 401 Unauthorized

{
    "error": {
        "code": 401,
        "message": "Request is missing required authentication credential. Expected OAuth 2 access token, login cookie
                    or other valid authentication credential.
                    See https://developers.google.com/identity/sign-in/web/devconsole-project.",
        "status": "UNAUTHENTICATED",
        "details": [
            {
                "@type": "type.googleapis.com/google.rpc.ErrorInfo",
                "reason": "CREDENTIALS_MISSING",
                "domain": "googleapis.com",
                "metadata": {
                    "method": "google.firebase.fcm.v1.FcmService.SendMessage",
                    "service": "fcm.googleapis.com"
                }
            }
        ]
    }
}
*/

// ----------------------------------------------------------------
// Response of Push Notification when oauth2.0 token is invalid
// ----------------------------------------------------------------

/*
HTTP response code: 401 Unauthorized

{
    "error": {
        "code": 401,
        "message": "Request had invalid authentication credentials. Expected OAuth 2 access token, login cookie or other
                    valid authentication credential.
                    See https://developers.google.com/identity/sign-in/web/devconsole-project.",
        "status": "UNAUTHENTICATED"
    }
}
*/

// ----------------------------------------------------------------
// Response of Push Notification when oauth2.0 token has expired
// ----------------------------------------------------------------

/*
HTTP response code: 401 Unauthorized

{
    "error": {
        "code": 401,
        "message": "Request had invalid authentication credentials. Expected OAuth 2 access token, login cookie or other
                    valid authentication credential. See
                    https://developers.google.com/identity/sign-in/web/devconsole-project.",
        "status": "UNAUTHENTICATED",
        "details": [
            {
                "@type": "type.googleapis.com/google.rpc.ErrorInfo",
                "reason": "ACCESS_TOKEN_EXPIRED",
                "domain": "googleapis.com",
                "metadata": {
                    "method": "google.firebase.fcm.v1.FcmService.SendMessage",
                    "service": "fcm.googleapis.com"
                }
            }
        ]
    }
}
*/

// ----------------------------------------------------------------
// Response of Push Notification when json payload is invalid
// ----------------------------------------------------------------

/*
HTTP response code: 400 Bad Request

{
    "error": {
        "code": 400,
        "message": "Invalid JSON payload received. Unknown name \"toen\" at 'message': Cannot find field.",
        "status": "INVALID_ARGUMENT",
        "details": [
            {
                "@type": "type.googleapis.com/google.rpc.BadRequest",
                "fieldViolations": [
                    {
                        "field": "message",
                        "description": "Invalid JSON payload received. Unknown name \"toen\" at 'message': Cannot find
                                        field."
                    }
                ]
            }
        ]
    }
}
*/

// ----------------------------------------------------------------
// Response of Push Notification when device token is missing
// ----------------------------------------------------------------

/*
HTTP response code: 400 Bad Request

{
    "error": {
        "code": 400,
        "message": "Recipient of the message is not set.",
        "status": "INVALID_ARGUMENT",
        "details": [
            {
                "@type": "type.googleapis.com/google.rpc.BadRequest",
                "fieldViolations": [
                    {
                        "field": "message",
                        "description": "Recipient of the message is not set."
                    }
                ]
            },
            {
                "@type": "type.googleapis.com/google.firebase.fcm.v1.FcmError",
                "errorCode": "INVALID_ARGUMENT"
            }
        ]
    }
}
*/

// ----------------------------------------------------------------
// Response of Push Notification when device token in invalid
// ----------------------------------------------------------------

/*
HTTP response code: 400 Bad Request

{
    "error": {
        "code": 400,
        "message": "The registration token is not a valid FCM registration token",
        "status": "INVALID_ARGUMENT",
        "details": [
            {
                "@type": "type.googleapis.com/google.firebase.fcm.v1.FcmError",
                "errorCode": "INVALID_ARGUMENT"
            }
        ]
    }
}
*/

// ----------------------------------------------------------------
// Response of Push Notification when device token is expired
// ----------------------------------------------------------------

/*
HTTP response code: 404 Not Found

{
    "error": {
        "code": 404,
        "message": "Requested entity was not found.",
        "status": "NOT_FOUND",
        "details": [
            {
                "@type": "type.googleapis.com/google.firebase.fcm.v1.FcmError",
                "errorCode": "UNREGISTERED"
            }
        ]
    }
}
*/

// ----------------------------------------------------------------
// Success Response of Push Notification
// ----------------------------------------------------------------

/*
HTTP response code: 200 OK

{
    "name" : "projects/matrix-ipvs---satatya-sight/messages/1717673451795380"
}
*/
