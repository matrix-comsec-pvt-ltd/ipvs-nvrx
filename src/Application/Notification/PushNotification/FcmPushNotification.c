//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		FcmPushNotification.c
@brief      This module Provides APIs to Send Push Notifications to Android/iOS devices
            using FCM service by Google.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
// Library Includes
#include <jansson.h>
#include <curl/curl.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

// Application Includes
#include "FcmPushNotification.h"
#include "ConfigApi.h"
#include "DebugLog.h"
#include "CameraInterface.h"
#include "EventLogger.h"
#include "Queue.h"
#include "Utils.h"
#include "DateTime.h"
#include "NetworkController.h"

//#################################################################################################
// @DEFINES
//#################################################################################################

//----------------------------------------------------------------
// Push Notification Module Tunable Parameters
//----------------------------------------------------------------

// maximum worker threads
#define NOTIFICATION_WORKER_THREAD_MAX      (1)

// software queue size
#define PUSH_NOTIFICATION_QUEUE_SIZE_MAX    (250)

// device inactivity timer in days
#define DEVICE_INACTIVITY_TIMER_DAYS        (10)

// fcm push notification payload size (4KB)
#define FCM_NOTIFY_PAYLOAD_LEN_MAX          (4 * 1024)

//----------------------------------------------------------------
// JWT token related defines
//----------------------------------------------------------------

// Token type
#define JWT_TYPE "JWT"

// JWT signing algorithm
#define JWT_SIGNING_ALGORITHM "RS256"

// JWT issuer
#define JWT_ISSUER "firebase-adminsdk-m7sen@matrix-ipvs---satatya-sight.iam.gserviceaccount.com"

// JWT API scope
#define JWT_SCOPE "https://www.googleapis.com/auth/firebase.messaging"

// JWT intended target of assertion
#define JWT_AUDIENCE "https://oauth2.googleapis.com/token"

// Private key aes256 encrypted base64 encoded
#define PRIVATE_KEY_AES256_BASE64                                                                                      \
    "LL8OgMZT1dkCgkPF7vqzBKo5hqwmBMVEOoMk+RGQcVzYRzOUbSeu5HW7Icn1EUQh8tM8VaAixCg1RQ20wL3nKXbAutzaEzptAWGXz+"           \
    "z2SiioMKf1LCxKDGeKl7mtpQmpfrs9ZDxSRkheY3sVjbycffrz8oqxDCkCFlOdMlTz6t6oSFZ9BlOOmiWQmELP97prhPHi8GK7UQhhuu74G4xLg+" \
    "+67ObNpCK8yOO5c4NlhHpBFCsK1sarWSfCBymd3NZjU0WT2QHslMAZaf+"                                                        \
    "sXlpHdkOTEu1mMdJqiBHcKFFeUhf6nJ3NNXDEPTkhmnelxAM9Oz5YGfwU9WWNZR1sJktz8/vsQKAbKlhNirN4rPf+q1XYV4Hjwdp/"            \
    "EXy9dZsPYf07PxCP3+OAAN6r74tdrkPYO+HyllbGJ9Ycl8y8rJSSvKAs64rTMZ9NuAK4LUO0iGzk5tXk8OEDET29OrV+CYlXeeQS4tNm3ekT73w/" \
    "oSDIRTOR7W5vMJ1biuKPRD3gp8GeuvnP+m2C5AzP8uA5b/"                                                                   \
    "M8tpJeKgnn3NgDfkKD9PbAyXpS4LsO6cbK23EgiHdKxmpEutpE3IZEjJMxD7w4F2+jRO3QdVusZdFE408jFhzDayV9na/"                    \
    "2Y73u6ZQ71NoKq5KVXnHwHC0w5Po+WKs3HzaXo5GK/"                                                                       \
    "PvkU4WzXKCcVB4lVQHKLdWWefnX+"                                                                                     \
    "b5ru1myLdtuKlwEkZkW3W5cZDJGrHnFumU742bwnIPRaB27PE5GBDsa3B6tE8GSE8v4v5jBPlkddsgOxUawAUhP9mNoyXqC0Ff5+E6+1E+"       \
    "U7HQW2AaNnsxbZ0TzLK+aoEPFqpdl49QWqjHKnzJP1mlJkTJlrqE2FJAaz8RxPsLVfGm60y4RyjMGS/1XOzjK81d3qs5D4R6qN/"              \
    "6+e2YDV7EWp6PK9n7qthSW+0TNTevDfK7HrupIm1C3Ar64/BIhk3mlBoAr8OMpdY3BxiILreO2yxx9epKK/"                              \
    "z28dUNpSVlNMRHqa8cSlrTwhjGKjIby0gekaz1hcMnMT/"                                                                    \
    "xSVef12eS7KDG9xT3Z3uxeRqB7hkjpMbtEduBUpZjIK4Tmis24mgaXD976vmdNt1PNG/9HojmNPWFzdijExen04qKsb9c5z3RVf4D/"           \
    "TAQb4W5gTsFwMY56V/g4/6RsKMq6QdEFl/FaL2HiT7rBqrlNK2CIqIbciMAeQe/"                                                  \
    "lCgZ4Ci9MwM4MDPNIqgXHDpgs4kACRWKTcDUfMdI1CH90bzwbt4bekdXEJHqAGh8aayYAfJnEH9shfza6OG50Xp7OtG6/"                    \
    "oe2heG03N7qO1eB4+68Q69osbuCrnz/"                                                                                  \
    "zGd0pVgJVU95ZHfwzuLKQFAe8CEodwQV06PFezR2xKp8RpRPMhivPb8lEVXoBntc8gepzQxCzLoDd8C3+"                                \
    "33gCQ6zvzOK7EOvAjbbcLrmsTuXiEQ6SphVKdg8TPsQUKHwvoCyoWguxJQMu49FOFIrFJHUztpSmhANmeFz7DOcyhXPyS520F5VMS3ws3UgqEti9" \
    "hkMFt04xnjt6bqTxL4ts4yKtVRZz0veKs3JFMdZNKrtVx0Uy1RCn+jvA/5yNSRIjzYtK7L+tWYbAZ4Q5k4fdoTfYKo0r/"                    \
    "Io4OhCt+"                                                                                                         \
    "w5TEezOdOLPCXmVZeOZzNnBerB69rEVMqYutvXupDkutbyc9Iq6a7Na04eojRURLA76SKa9kybrDzScZUAOF8SO2dwZI4ZMnHKMME3aFm7WFVp1W" \
    "C7iN1OOdPlPKO/"                                                                                                   \
    "TqC04On1ii1Nv15OiEOyBzXuIFlaBhtIGKPZiJhNlS8EUqVp5z8BEnfZsTpvBIB3IaaYlLkwiaKfz3XfZ0lH0vt5XALBTs1PZWMWWeP4dVMRizLt" \
    "eHwWq5GjWS5sG0kFW1fde3vxy7SaaBSs1t1DRCdpY2YFj+rRcl2jKB4uno1L7KaZVMM9RcMpzL8QrnJs4z3535w1yyyg2de/"                 \
    "6MaJ14dnrGSNL5kHTnfFyEw/v9cHhGqdOqV2VEMw5SdYgX1N/8d9qvfUmh9jR9l4mK/"                                              \
    "c50hbKEZhegqmHvJc8fs8vyxxONBTUTHDVJfZomOHZ2wdQ/mVeJ/tAFb2iYjZafw/"                                                \
    "o4fs+bndN6wMXa9V3QJ57moDGAtHAGrob3ymQHrwhmxJPQ06nlspf1zkGN8B2/"                                                   \
    "4xJiP04SWF8z0auztOJHeqP1n8qb5mGdTB7TKUMQzwstbMh2SMyDpNTefiAilQMH6UG2MwbXJ03grAUVSPLojlbkgtwShrX+"                 \
    "anFAnn8VOTbqM94FXfZFvJlrLAT4EKrjsJzH2kNxojPKjCL7CI="

// JWT token maximum length
#define JWT_TOKEN_LENGTH (2048)

// JWT token payload size
#define JWT_JSON_PAYLOAD_SIZE (1024)

// Private key length
#define PRIVATE_KEY_LENGTH (2048)

//----------------------------------------------------------------
// Get oauth2 access token related defines
//----------------------------------------------------------------

// Endpoint URL to request oauth2 access token
#define OAUTH_TOKEN_REQUEST_ENDPOINT "https://oauth2.googleapis.com/token"

// oauth2 access token request content type
#define OAUTH_TOKEN_REQ_CONTENT_TYPE "Content-Type: application/x-www-form-urlencoded"

// oauth2 access token request data
#define OAUTH_TOKEN_REQ_POST_DATA "grant_type=urn:ietf:params:oauth:grant-type:jwt-bearer&assertion=%s"

// oauth2 token length
#define OAUTH_TOKEN_LENGTH (2048)

//----------------------------------------------------------------
// Send push notification related defines
//----------------------------------------------------------------

// Application project id
#define FCM_APP_PROJECT_ID "matrix-ipvs---satatya-sight"

// Endpoint URL to send push notification
#define PUSH_NOTIFICATION_REQUEST_ENDPOINT "https://fcm.googleapis.com/v1/projects/" FCM_APP_PROJECT_ID "/messages:send"

// Push notification request content type
#define PUSH_NOTIFICATION_REQ_CONTENT_TYPE "Content-Type: application/json; charset=UTF-8"

// Push notification authorization
#define PUSH_NOTIFICATION_REQ_AUTHORIZATION "Authorization: Bearer %s"

// Push notification request payload
#define PUSH_NOTIFICATION_PAYLOAD_DATA                             \
    "{"                                                            \
    "\n    \"message\": {"                                         \
    "\n        \"token\": \"%s\","                                 \
    "\n        \"notification\": {"                                \
    "\n            \"title\": \"%s\","                             \
    "\n            \"body\": \"%s\""                               \
    "\n        },"                                                 \
    "\n        \"data\": {"                                        \
    "\n            \"date_time\": \"%s\","                         \
    "\n            \"nvr_mac\": \"%s\""                            \
    "\n        },"                                                 \
    "\n        \"android\": {"                                     \
    "\n            \"notification\": {"                            \
    "\n                \"click_action\": \"notification_search\""  \
    "\n            }"                                              \
    "\n        },"                                                 \
    "\n        \"apns\": {"                                        \
    "\n            \"headers\": {"                                 \
    "\n                \"apns-priority\": \"5\""                   \
    "\n            },"                                             \
    "\n            \"payload\": {"                                 \
    "\n                \"aps\": {"                                 \
    "\n                    \"category\": \"notification_search\"," \
    "\n                    \"content-available\": 1"               \
    "\n                }"                                          \
    "\n            }"                                              \
    "\n        }"                                                  \
    "\n    }"                                                      \
    "\n}"

//----------------------------------------------------------------
// Curl Timeouts
//----------------------------------------------------------------

#define CURLOPT_TCP_CONN_TIMEOUT_SECONDS (5L)

#define CURLOPT_OPERATION_TIMEOUT_SECONDS (10L)

#define CULROPT_LOW_SPEED_LIMIT (100L)

#define CULROPT_LOW_SPEED_TIME (4L)

//----------------------------------------------------------------
// Other Defines
//----------------------------------------------------------------
/* Delay after which to process notification on boot.
 * Otherwise boot notification will be failed as internet
 * connectivity is not immediately active on boot.*/
#define NOTIFICATION_TIMER_WAIT_ON_BOOT_SEC             (30)

// sleep when there is no data in the queue (1 sec)
#define NOTIFICATION_THREAD_SLEEP_USEC                  (1000000)

//#################################################################################################
// @ENUMS
//#################################################################################################

/**
 * @brief error codes return by FCM server
 */
typedef enum
{
    /*
     * SUCCESS (HTTP 200 OK)
     */
    FCM_NO_ERROR                              = 200,

    /*
     * INVALID_ARGUMENT (HTTP error code = 400)
     * Request parameters were invalid. An extension of type google.rpc.BadRequest is
     * returned to specify which field was invalid.
     */
    FCM_ERROR_INVALID_ARGUMENT                = 400,

    /*
     * THIRD_PARTY_AUTH_ERROR (HTTP error code = 401)
     * APNs certificate or web push auth key was invalid or missing.
     */
    FCM_ERROR_AUTHENTICATION                  = 401,

    /*
     * SENDER_ID_MISMATCH (HTTP error code = 403)
     * The authenticated sender ID is different from the sender ID for the registration token.
     */
    FCM_ERROR_SENDER_ID_MISMATCH              = 403,

    /*
     * UNREGISTERED (HTTP error code = 404)
     * App instance was unregistered from FCM. This usually means that the token
     * used is no longer valid and a new one must be used.
     */
    FCM_ERROR_UNREGISTERED                    = 404,

    /*
     * QUOTA_EXCEEDED (HTTP error code = 429)
     * Sending limit exceeded for the message target. An extension of type
     * google.rpc.QuotaFailure is returned to specify which quota was exceeded.
     */
    FCM_ERROR_QUOTA_EXCEEDED                  = 429,

    /*
     * INTERNAL (HTTP error code = 500) An unknown internal error occurred.
     */
    FCM_RESPONSE_ERR_INTERNAL_SERVER_ERR      = 500,

    /*
     * UNAVAILABLE (HTTP error code = 503) The server is overloaded.
     */
    FCM_RESPONSE_ERR_SERVER_UNAVAILABLE       = 503

} FCM_ERROR_CODE_e;

//#################################################################################################
// @DATA TYPES
//#################################################################################################

/**
 * @brief JWT header parameters
 *
 */
typedef struct
{
    CHAR algorithm[10];
    CHAR type[10];

} JWT_HEADER_t;

/**
 * @brief JWT payload parameters
 *
 */
typedef struct
{
    CHAR issuer[256];
    CHAR scope[256];
    CHAR audience[256];
    time_t now;
    time_t exp;

} JWT_PAYLOAD_t;

/**
 * @brief JWT token generation data
 */
typedef struct
{
    JWT_HEADER_t header;
    JWT_PAYLOAD_t payload;

} JWT_DATA_t;

/**
 * @brief push notification entry in the notification queue
 */
typedef struct
{
    CHAR title[FCM_PAYLOAD_ELEMENT_LEN_MAX];
    CHAR body[FCM_PAYLOAD_ELEMENT_LEN_MAX];
    CHAR dateTime[FCM_PAYLOAD_ELEMENT_LEN_MAX];

} PUSH_NOTIFICATION_QUEUE_ENTRY_t;

/**
 * @brief status of each device whether to send push notification or not
 */
typedef struct
{
    UINT8 sessionIndex;
    BOOL notificationEnabled;
    UINT64 deviceExpiryTimestamp;
    FCM_PUSH_NOTIFY_CONFIG_t deviceConfig;

} PUSH_NOTIFY_SESSION_PARAM_t;

/**
 * @brief push notification payload parameters
 */
typedef struct
{
    CHAR title[FCM_PAYLOAD_ELEMENT_LEN_MAX];
    CHAR body[FCM_PAYLOAD_ELEMENT_LEN_MAX];
    CHAR dateTime[FCM_PAYLOAD_ELEMENT_LEN_MAX];
    CHAR lan1MacAddr[MAX_MAC_ADDRESS_WIDTH];

} NOTIFICATION_PAYLOAD_PARAM_t;

/**
 * @brief holds curl response payload parameters
 */
typedef struct
{
    // response payload string dynamic memory
    CHARPTR pPayloadString;

    // response payload size
    size_t payloadSize;

    // curl response code
    INT32 responseCode;

} CURL_RESPONSE_PAYLOAD_t;

/**
 * @brief holds FCM server response payload parameters
 */
typedef struct
{
    // main http response code
    INT32 curlResponseCode;

    // count of success tokens
    UINT8 successCnt;

    // count of failure tokens
    UINT8 failureCnt;

    // derived sub response code for each token
    INT32 perTokenResponseCode[FCM_PUSH_NOTIFY_DEVICES_MAX];

} FCM_SERVER_RESPONSE_PAYLOAD_t;

/**
 * @brief structure to hold oauth token parameters
 */
typedef struct
{
    CHAR accessToken[OAUTH_TOKEN_LENGTH];
    UINT32 expiresInSeconds;

} OAUTH_TOKEN_PARAM_t;

/**
 * @brief push notification thread related paramertes
 */
typedef struct
{
    BOOL exitGuard;
    pthread_t notificationThreadId;

} NOTIFICATION_THREAD_PARAM_t;

//#################################################################################################
// @VARIABLES
//#################################################################################################

// notification queue handle
static QUEUE_HANDLE gNotificationQueueHndl = NULL;

// number of threads
static NOTIFICATION_THREAD_PARAM_t gNotificationWorkerThread[NOTIFICATION_WORKER_THREAD_MAX];

// structure to hold user session info
static pthread_mutex_t gSessionParamMutex = PTHREAD_MUTEX_INITIALIZER;
static PUSH_NOTIFY_SESSION_PARAM_t gNotificationSessionParam[FCM_PUSH_NOTIFY_DEVICES_MAX];

// timer handle
static TIMER_HANDLE gProcessWaitTimerHandle = INVALID_TIMER_HANDLE;
static BOOL gStartPushNotification = FALSE;

// save oauth token to be used till it gets expired
static OAUTH_TOKEN_PARAM_t gOauthTokenParam = {"", 0};

// service account private key
static CHAR gServiceAccountPkey[PRIVATE_KEY_LENGTH] = "";

// decryption password (HiddenPassword)
static const CHAR gPkeyDecryptPassword[] = {0x48, 0x69, 0x64, 0x64, 0x65, 0x6E, 0x50, 0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64, 0x00};

// decryption salt key (Penguine)
static const CHAR gPkeyDecryptSalt[] = {0x50, 0x65, 0x6E, 0x67, 0x75, 0x69, 0x6E, 0x65, 0x00};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void PushNotificationQueueFullCb(VOIDPTR entry);
//-------------------------------------------------------------------------------------------------
static BOOL CheckPushNotificationDeviceExpiryTimer(UINT32 data);
//-------------------------------------------------------------------------------------------------
static BOOL PrepareQueueEntry(PUSH_NOTIFICATION_EVENT_PARAM_t *pEventParam, PUSH_NOTIFICATION_QUEUE_ENTRY_t *pQueueEntry);
//-------------------------------------------------------------------------------------------------
static VOIDPTR PushNotificationThread(VOIDPTR threadParam);
//-------------------------------------------------------------------------------------------------
static void DestroyWorkerThreads(void);
//-------------------------------------------------------------------------------------------------
static BOOL StartNotificationProcessWaitTimer(UINT16 timeInSeconds);
//-------------------------------------------------------------------------------------------------
static void EnablePushNotificationProcess(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void InitNotificationSessionParam(void);
//-------------------------------------------------------------------------------------------------
static void DefaultNotificationSessionParam(void);
//-------------------------------------------------------------------------------------------------
static void DefaultNotificationSingleSessionParam(UINT8 deviceIndex);
//-------------------------------------------------------------------------------------------------
static void UpdateSessionParameters(UINT8 deviceIndex, UINT8 sessionIndex, BOOL enablePushNotification);
//-------------------------------------------------------------------------------------------------
static void RemoveAllDeviceOfUserForNotification(CHARPTR username);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e EnableDevicePushNotification(UINT8 sessionIndex, CHARPTR fcmToken, CHARPTR deviceModelname);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e DisableDevicePushNotification(CHARPTR fcmToken);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e SaveNewDeviceForNotification(UINT8 sessionIndex, CHARPTR username, CHARPTR fcmToken, CHARPTR deviceModelname);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e RemoveDeviceForNotification(UINT8 deviceIndex);
//-------------------------------------------------------------------------------------------------
static UINT8 GetTotalNotificationDeviceCount(void);
//-------------------------------------------------------------------------------------------------
static UINT8 GetFreeDeviceIndex(void);
//-------------------------------------------------------------------------------------------------
static UINT8 GetDeviceIndexFromSessionIndex(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
static UINT8 GetDeviceIndexFromFcmToken(CHARPTR fcmToken);
//-------------------------------------------------------------------------------------------------
static UINT64 GetDeviceExpiryTime(void);
//-------------------------------------------------------------------------------------------------
static void PerformPushNotificationRequest(PUSH_NOTIFICATION_QUEUE_ENTRY_t *pQueueEntry);
//-------------------------------------------------------------------------------------------------
static void ProcessFcmErrorResponse(FCM_ERROR_CODE_e fcmResponse, const CHARPTR deviceToken, BOOL *pFatalError, BOOL *pRetryNotification);
//-------------------------------------------------------------------------------------------------
static CHAR *Base64UrlEncode(const UINT8 *input, size_t len);
//-------------------------------------------------------------------------------------------------
static void SetCurlDefaultOptions(CURL *handle);
//-------------------------------------------------------------------------------------------------
static size_t CurlWiteDataCallback(void *pContents, size_t size, size_t nmemb, void *userData);
//-------------------------------------------------------------------------------------------------
static BOOL DecryptServiceAccountPkey(void);
//-------------------------------------------------------------------------------------------------
static BOOL GenerateJwtToken(CHARPTR jwtToken);
//-------------------------------------------------------------------------------------------------
static BOOL RefreshOauthAccessToken(OAUTH_TOKEN_PARAM_t *pOauthTokenParam);
//-------------------------------------------------------------------------------------------------
static BOOL OauthTokenHttpRequest(const CHARPTR pJwtToken, CURL_RESPONSE_PAYLOAD_t *pCurlResponse);
//-------------------------------------------------------------------------------------------------
static BOOL OauthTokenPraseHttpResponse(CURL_RESPONSE_PAYLOAD_t *pCurlResponse, OAUTH_TOKEN_PARAM_t *pOauthTokenParam);
//-------------------------------------------------------------------------------------------------
static void PrepareDeviceTokenList(CHAR deviceTokenList[][FCM_TOKEN_LENGTH_MAX], UINT8 *pTokenCount);
//-------------------------------------------------------------------------------------------------
static BOOL PrepareFcmNotificationData(PUSH_NOTIFICATION_QUEUE_ENTRY_t *pQueueEntry, NOTIFICATION_PAYLOAD_PARAM_t *pPayloadParam);
//-------------------------------------------------------------------------------------------------
static BOOL PrepareFcmNotificationJsonPayload(NOTIFICATION_PAYLOAD_PARAM_t *pPayloadParam, CHARPTR deviceToken, CHARPTR pJsonPayloadStr);
//-------------------------------------------------------------------------------------------------
static BOOL PushNotificationHttpRequest(CHARPTR pJsonPayload, CHARPTR pOauthToken, CURL_RESPONSE_PAYLOAD_t *pCurlResponse);
//-------------------------------------------------------------------------------------------------
static BOOL PushNotificationParseHttpResponse(CURL_RESPONSE_PAYLOAD_t *pCurlResponse, FCM_ERROR_CODE_e *pFcmResponse);
//-------------------------------------------------------------------------------------------------
static BOOL PerformPushNotification(CHARPTR pDeviceToken, CHARPTR pOauthToken, NOTIFICATION_PAYLOAD_PARAM_t *pPayload, FCM_ERROR_CODE_e *pFcmResponse);
//-------------------------------------------------------------------------------------------------
static BOOL JSON_GET_INTEGER(json_t *json_root, json_t *json_current, CHAR *tag, json_int_t *value);
//-------------------------------------------------------------------------------------------------
static BOOL JSON_GET_STRING(json_t *json_root, json_t *json_current, CHAR *tag, CHAR *buff, INT32 buff_size);
//-------------------------------------------------------------------------------------------------
#if 0
static BOOL JSON_GET_OBJECT(json_t *json_root, json_t **json_obj, CHAR *tag);
static BOOL JSON_GET_ARRAY(json_t *json_root, json_t **json_array, CHAR *tag);
#endif

//#################################################################################################
// @FUNCTIONS
//#################################################################################################

/**
 * @brief   Initialize push notification module with email queue and creates thread for sending notification
 * @return  SUCCESS/FAIL
 */
BOOL InitFcmPushNotification(void)
{
    QUEUE_INIT_t notificationQueue;
    UINT8 threadCnt;
    ONE_MIN_NOTIFY_t oneMinuteTask;

    // start initial timer to wait to process notification
    if (SUCCESS != StartNotificationProcessWaitTimer(NOTIFICATION_TIMER_WAIT_ON_BOOT_SEC))
    {
        EPRINT(PUSH_NOTIFY, "fail to create notification process wait timer");
        return FAIL;
    }

    // Create a Queue for push notification
    QUEUE_PARAM_INIT(notificationQueue);

    notificationQueue.maxNoOfMembers = PUSH_NOTIFICATION_QUEUE_SIZE_MAX;
    notificationQueue.sizoOfMember   = sizeof(PUSH_NOTIFICATION_QUEUE_ENTRY_t);
    notificationQueue.callback = PushNotificationQueueFullCb;

    gNotificationQueueHndl = QueueCreate(&notificationQueue);

    // Decrypt serice account private key which is stored using aes256 encryption
    if (TRUE != DecryptServiceAccountPkey())
    {
        return FAIL;
    }

    // reset data of thread descriptor
    memset(gNotificationWorkerThread, 0, sizeof(gNotificationWorkerThread));

    // create worker threads to send push notifications
    for (threadCnt = 0; threadCnt < NOTIFICATION_WORKER_THREAD_MAX; threadCnt++)
    {
        gNotificationWorkerThread[threadCnt].exitGuard = FALSE;

        if (FAIL == Utils_CreateThread(&gNotificationWorkerThread[threadCnt].notificationThreadId,
                                       PushNotificationThread, &gNotificationWorkerThread[threadCnt],
                                       JOINABLE_THREAD, 0))
        {
            gNotificationWorkerThread[threadCnt].exitGuard = TRUE;
            EPRINT(PUSH_NOTIFY, "fail to create push notification thread");

            // stop threads & destroy queue
            DeinitFcmPushNotification();

            return FAIL;
        }
    }

    // register 1 minute task to check for device expiry timer
    oneMinuteTask.funcPtr = CheckPushNotificationDeviceExpiryTimer;
    oneMinuteTask.userData = 0;

    if (RegisterOnMinFun(&oneMinuteTask) != SUCCESS)
    {
        EPRINT(PUSH_NOTIFY, "fail to start push notification device monitor task");

        // stop threads & destroy queue
        DeinitFcmPushNotification();

        return FAIL;
    }

    // enable push notification for all saved device by default
    InitNotificationSessionParam();

    DPRINT(PUSH_NOTIFY, "push notification module init successfully");

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function frees up resources used by this module and if thread is sleep state the
 *          wake up the thread and sends terminate signal to thread.
 * @return  SUCCESS/FAIL
 */
BOOL DeinitFcmPushNotification(void)
{
    DPRINT(PUSH_NOTIFY, "destroy worker threads");

    // destroy all worker threads & wait till they exit
    DestroyWorkerThreads();

    // default all device session pameters
    DefaultNotificationSessionParam();

    // destroy notification queue
    QueueDestroy(gNotificationQueueHndl);

    DPRINT(PUSH_NOTIFY, "push notification module deinit successfully");

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief FcmPushNotifyMobileUserLogin
 * @param sessionIndex
 * @param username
 * @param fcmToken
 */
void FcmPushNotifyMobileUserLogin(UINT8 sessionIndex, CHARPTR username, CHARPTR fcmToken)
{
    UINT8 deviceIndex;

    if ((NULL == username) || (NULL == fcmToken))
    {
        return;
    }

    MUTEX_LOCK(gSessionParamMutex);

    // get device index from fcm token
    deviceIndex = GetDeviceIndexFromFcmToken(fcmToken);

    if (deviceIndex >= FCM_PUSH_NOTIFY_DEVICES_MAX)
    {
        EPRINT(PUSH_NOTIFY, "device not found: user login");
        MUTEX_UNLOCK(gSessionParamMutex);
        return;
    }

    /* special case: if token matched but username doesn't mached, remove the device
     * It will happen when new user logged in from previously added device.
     */
    if ((strcmp(fcmToken, gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken) == STATUS_OK)
            && (strcmp(username, gNotificationSessionParam[deviceIndex].deviceConfig.username) != STATUS_OK))
    {
        EPRINT(PUSH_NOTIFY, "remove device: login username & device token mismatch: [oldUsername=%s], [newUsername=%s], [fcmToken=%s]",
               gNotificationSessionParam[deviceIndex].deviceConfig.username, username,
               gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken);

        // remove the entry as other user is logging from same device. first user will need to re-enable push again.
        RemoveDeviceForNotification(deviceIndex);
        MUTEX_UNLOCK(gSessionParamMutex);

        return;
    }

    // same user is logged in with same device. so disable push notification for the device
    UpdateSessionParameters(deviceIndex, sessionIndex, FALSE);

    DPRINT(PUSH_NOTIFY, "push notification disabled: user login: [username=%s], [modelName=%s], [fcmToken=%s]",
           gNotificationSessionParam[deviceIndex].deviceConfig.username,
           gNotificationSessionParam[deviceIndex].deviceConfig.deviceModelName,
           gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken);

    MUTEX_UNLOCK(gSessionParamMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief FcmPushNotifyMobileUserLogout
 * @param sessionIndex
 */
void FcmPushNotifyMobileUserLogout(UINT8 sessionIndex)
{
    UINT8 deviceIndex;

    MUTEX_LOCK(gSessionParamMutex);

    // check if device is added in the list or not
    deviceIndex = GetDeviceIndexFromSessionIndex(sessionIndex);

    // return if device not found
    if (deviceIndex >= FCM_PUSH_NOTIFY_DEVICES_MAX)
    {
        MUTEX_UNLOCK(gSessionParamMutex);
        return;
    }

    // enable push notification for the device
    UpdateSessionParameters(deviceIndex, INVALID_SESSION_INDEX, TRUE);

    DPRINT(PUSH_NOTIFY, "push notification enabled: user logout: [username=%s], [modelName=%s], [fcmToken=%s]",
           gNotificationSessionParam[deviceIndex].deviceConfig.username,
           gNotificationSessionParam[deviceIndex].deviceConfig.deviceModelName,
           gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken);

    MUTEX_UNLOCK(gSessionParamMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief FcmPushDeleteSystemUserNotify
 * @param username
 */
void FcmPushDeleteSystemUserNotify(CHARPTR username)
{
    if (NULL == username)
    {
        return;
    }

    DPRINT(PUSH_NOTIFY, "remove all devices: delete/block user: [username=%s]", username);

    // search for username in the configuration & delete all associated device
    RemoveAllDeviceOfUserForNotification(username);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ManagePushNotificationSts
 * @param sessionIndex
 * @param enable
 * @param username
 * @param fcmToken
 * @param deviceModelname
 * @return
 */
NET_CMD_STATUS_e ManagePushNotificationSts(UINT8 sessionIndex, BOOL enable, CHARPTR fcmToken, CHARPTR deviceModelname)
{
    NET_CMD_STATUS_e ret;

    if ((NULL == fcmToken) || (NULL == deviceModelname))
    {
        return CMD_MAX_USER_SESSION;
    }

    if (TRUE == enable)
    {
        // request to add/update entry in the notification table
        ret = EnableDevicePushNotification(sessionIndex, fcmToken, deviceModelname);
    }
    else
    {
        // request to remove entry from the notification table
        ret = DisableDevicePushNotification(fcmToken);
    }

    return ret;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetDevicePushNotificationEnableSts
 * @param fcmToken
 * @return
 */
NET_CMD_STATUS_e GetDevicePushNotificationEnableSts(CHARPTR fcmToken)
{
    UINT8 deviceIndex;
    NET_CMD_STATUS_e status = CMD_SUCCESS;

    if (NULL == fcmToken)
    {
        return CMD_PUSH_NOTIFICATION_DISABLED;
    }

    MUTEX_LOCK(gSessionParamMutex);

    // get device index from session index
    deviceIndex = GetDeviceIndexFromFcmToken(fcmToken);

    // device not found
    if (deviceIndex >= FCM_PUSH_NOTIFY_DEVICES_MAX)
    {
        DPRINT(PUSH_NOTIFY, "get push notification status: [sts=disabled], [fcmToken=%s]", fcmToken);
        status = CMD_PUSH_NOTIFICATION_DISABLED;
    }
    else
    {
        DPRINT(PUSH_NOTIFY, "get push notification status: [sts=enabled], [fcmToken=%s]", fcmToken);
    }

    MUTEX_UNLOCK(gSessionParamMutex);

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief FcmPushNotifyConfigUpdate
 * @param newCopy
 * @param oldCopy
 * @return
 */
BOOL FcmPushNotifyConfigUpdate(FCM_PUSH_NOTIFY_CONFIG_t newCopy, FCM_PUSH_NOTIFY_CONFIG_t *oldCopy)
{
    UINT8 deviceIndex;

    if (NULL == oldCopy)
    {
        return FALSE;
    }

    // disable push notificaion status flag if any device is deleted by user from UI
    if ((oldCopy->deviceFcmToken[0] != '\0') && (newCopy.deviceFcmToken[0] == '\0'))
    {
        deviceIndex = GetDeviceIndexFromFcmToken(oldCopy->deviceFcmToken);

        if (deviceIndex < FCM_PUSH_NOTIFY_DEVICES_MAX)
        {
            DPRINT(PUSH_NOTIFY, "remove device: configuration update: [username=%s], [modelName=%s], [fcmToken=%s]",
               oldCopy->username, oldCopy->deviceModelName, oldCopy->deviceFcmToken);

            // default configuration & session parameters
            RemoveDeviceForNotification(deviceIndex);
        }
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Prepare push notification payload & add it to queue
 * @param pEventParam
 * @return
 */
NET_CMD_STATUS_e ProcessFcmPushNotification(PUSH_NOTIFICATION_EVENT_PARAM_t *pEventParam)
{
    PUSH_NOTIFICATION_QUEUE_ENTRY_t queueEntry;

    if (NULL == pEventParam)
    {
        return CMD_PROCESS_ERROR;
    }

    // module is still not initialized
    if (NULL == gNotificationQueueHndl)
    {
        return CMD_PROCESS_ERROR;
    }

    MUTEX_LOCK(gSessionParamMutex);

    // no need to process push notificaiton if no active devices are present
    if (0 == GetTotalNotificationDeviceCount())
    {
        MUTEX_UNLOCK(gSessionParamMutex);
        return CMD_SUCCESS;
    }

    MUTEX_UNLOCK(gSessionParamMutex);

    // prepare queue entry
    if (TRUE != PrepareQueueEntry(pEventParam, &queueEntry))
    {
        EPRINT(PUSH_NOTIFY, "discard push notification: fail to prepare notification queue entry");
        return CMD_PROCESS_ERROR;
    }

    // insert notification entry with data in the queue
    if (FAIL == QueueAddEntry(gNotificationQueueHndl, (VOIDPTR)&queueEntry))
    {
        EPRINT(PUSH_NOTIFY, "discard push notification: fail to add in queue");
        return CMD_RESOURCE_LIMIT;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetPushNotificationDeviceStsList
 * @param pDeviceList
 * @return
 */
NET_CMD_STATUS_e GetPushNotificationDeviceStsList(PUSH_NOTIFY_DEVICE_STS_LIST_t *pDeviceList)
{
    UINT8 deviceIndex;

    if (NULL == pDeviceList)
    {
        return CMD_PROCESS_ERROR;
    }

    time_t currentTimeSinceEpoch;

    // get current time
    GetLocalTimeInSec(&currentTimeSinceEpoch);

    MUTEX_LOCK(gSessionParamMutex);

    for (deviceIndex = 0; deviceIndex < FCM_PUSH_NOTIFY_DEVICES_MAX; deviceIndex++)
    {
        // token will not be displayed on status but used for delete device command
        snprintf(pDeviceList[deviceIndex].deviceFcmToken, sizeof(pDeviceList[deviceIndex].deviceFcmToken), "%s",
                gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken);

        // username
        snprintf(pDeviceList[deviceIndex].username, sizeof(pDeviceList[deviceIndex].username), "%s",
                gNotificationSessionParam[deviceIndex].deviceConfig.username);

        // device model name
        snprintf(pDeviceList[deviceIndex].deviceModelName, sizeof(pDeviceList[deviceIndex].deviceModelName), "%s",
                gNotificationSessionParam[deviceIndex].deviceConfig.deviceModelName);

        pDeviceList[deviceIndex].deviceInactivityTimer = 0;

        // derive remaining time to expire from stored timestamp since epoch
        if (gNotificationSessionParam[deviceIndex].deviceExpiryTimestamp > (UINT64)currentTimeSinceEpoch)
        {
            pDeviceList[deviceIndex].deviceInactivityTimer =
                    (gNotificationSessionParam[deviceIndex].deviceExpiryTimestamp - currentTimeSinceEpoch);
        }

        // in case of last minute is remaining, consider it as 60 sec because we only display HHH:mm on status page
        if ((gNotificationSessionParam[deviceIndex].deviceExpiryTimestamp > 0) && (pDeviceList[deviceIndex].deviceInactivityTimer < 60))
        {
            pDeviceList[deviceIndex].deviceInactivityTimer = 60;
        }
    }

    MUTEX_UNLOCK(gSessionParamMutex);

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief DeletePushNotificationDevice
 * @param fcmToken
 * @return
 */
NET_CMD_STATUS_e DeletePushNotificationDevice(CHARPTR fcmToken)
{
    UINT8 deviceIndex;
    NET_CMD_STATUS_e ret = CMD_PROCESS_ERROR;

    if (NULL == fcmToken)
    {
        return CMD_PROCESS_ERROR;
    }

    MUTEX_LOCK(gSessionParamMutex);

    // get device index from fcm token
    deviceIndex = GetDeviceIndexFromFcmToken(fcmToken);

    if (deviceIndex >= FCM_PUSH_NOTIFY_DEVICES_MAX)
    {
        EPRINT(PUSH_NOTIFY, "fail to remove device: user delete request: device not found: [fcmToken=%s]", fcmToken);
    }
    else
    {
        DPRINT(PUSH_NOTIFY, "remove device: user delete request: [username=%s], [modelName=%s], [fcmToken=%s]",
               gNotificationSessionParam[deviceIndex].deviceConfig.username,
               gNotificationSessionParam[deviceIndex].deviceConfig.deviceModelName,
               gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken);

        // remove device from configuration
        RemoveDeviceForNotification(deviceIndex);

        ret = CMD_SUCCESS;
    }

    MUTEX_UNLOCK(gSessionParamMutex);

    return ret;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This callback is called when queue is full, and the entry is being over written.
 * @param   entry
 * @return
 */
static void PushNotificationQueueFullCb(VOIDPTR entry)
{
    EPRINT(PUSH_NOTIFY, "push notification queue is full");
    WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Push Notification Queue Full", EVENT_FAIL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CheckPushNotificationDeviceExpiryTimer
 * @param data
 * @return
 */
static BOOL CheckPushNotificationDeviceExpiryTimer(UINT32 data)
{
    UINT8 deviceIndex;
    time_t currentTimeSinceEpoch;

    // get current time
    GetLocalTimeInSec(&currentTimeSinceEpoch);

    MUTEX_LOCK(gSessionParamMutex);

    for (deviceIndex = 0; deviceIndex < FCM_PUSH_NOTIFY_DEVICES_MAX; deviceIndex++)
    {
        // check for active push notification devices
        if (FALSE == gNotificationSessionParam[deviceIndex].notificationEnabled)
        {
            continue;
        }

        // device has not done login since DEVICE_INACTIVITY_TIMER_DAYS, so remove device
        if ((UINT64)currentTimeSinceEpoch >= gNotificationSessionParam[deviceIndex].deviceExpiryTimestamp)
        {
            DPRINT(PUSH_NOTIFY, "remove device: inactivity timer expired: [username=%s], [modelName=%s], [fcmToken=%s]",
                   gNotificationSessionParam[deviceIndex].deviceConfig.username,
                   gNotificationSessionParam[deviceIndex].deviceConfig.deviceModelName,
                   gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken);

            RemoveDeviceForNotification(deviceIndex);
        }
    }

    MUTEX_UNLOCK(gSessionParamMutex);

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief PrepareQueueEntry
 * @param pEventParam
 * @param pQueueEntry
 * @return
 */
static BOOL PrepareQueueEntry(PUSH_NOTIFICATION_EVENT_PARAM_t *pEventParam, PUSH_NOTIFICATION_QUEUE_ENTRY_t *pQueueEntry)
{
    if ((NULL == pEventParam) || (NULL == pQueueEntry))
    {
        return FALSE;
    }

    // copy data
    snprintf(pQueueEntry->title, sizeof(pQueueEntry->title), "%s", pEventParam->title);
    snprintf(pQueueEntry->body, sizeof(pQueueEntry->body), "%s", pEventParam->body);
    snprintf(pQueueEntry->dateTime, sizeof(pQueueEntry->dateTime), "%s", pEventParam->dateTime);

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is main routine of thread which sends push notification
 * @param   threadParam
 * @return
 */
static VOIDPTR PushNotificationThread(VOIDPTR threadParam)
{
    NOTIFICATION_THREAD_PARAM_t *pThreadParam = ((NOTIFICATION_THREAD_PARAM_t*)threadParam);
    PUSH_NOTIFICATION_QUEUE_ENTRY_t *pQueueEntry;

    THREAD_START("PUSH_NOTIFY");

    // check for therad exit signal
    while (!pThreadParam->exitGuard)
    {
        // on boot push notification process will be delayed to get the network up
        if (FALSE == gStartPushNotification)
        {
            // wait for start signal
            sleep(1);
            continue;
        }

        // get single notification entry from queue
        pQueueEntry = QueueGetAndFreeEntry(gNotificationQueueHndl, FALSE);

        if (NULL == pQueueEntry)
        {
            // sleep for some time
            usleep(NOTIFICATION_THREAD_SLEEP_USEC);
            continue;
        }

        // process push notification entry from the queue
        do
        {
            /* no need to process push notificaiton if no active devices are present.
             * it is possible that admin has deleted all devices from table.
             */
            MUTEX_LOCK(gSessionParamMutex);

            if (0 == GetTotalNotificationDeviceCount())
            {
                EPRINT(PUSH_NOTIFY, "fail to process notification: no device: [title=%s], [body=%s]", pQueueEntry->title, pQueueEntry->body);
                MUTEX_UNLOCK(gSessionParamMutex);
                break;
            }

            MUTEX_UNLOCK(gSessionParamMutex);

            // if internet is not connected, discard notification
            if (getInternetConnStatus() != ACTIVE)
            {
                EPRINT(PUSH_NOTIFY, "fail to process notification: no internet: [title=%s], [body=%s]", pQueueEntry->title, pQueueEntry->body);
                break;
            }

            // perform http post request using curl library
            PerformPushNotificationRequest(pQueueEntry);

        } while(0);

        // free notification queue data pointer
        free(pQueueEntry);
    }

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief DestroyWorkerThreads
 */
static void DestroyWorkerThreads(void)
{
    UINT8 threadCnt;

    for (threadCnt = 0; threadCnt < NOTIFICATION_WORKER_THREAD_MAX; threadCnt++)
    {
        if (0 == gNotificationWorkerThread[threadCnt].notificationThreadId)
        {
            continue;
        }

        // signal threads to exit
        gNotificationWorkerThread[threadCnt].exitGuard = TRUE;

        // join thread to release memory
        pthread_join(gNotificationWorkerThread[threadCnt].notificationThreadId, NULL);
    }

    // reset data of thread descriptor
    memset(gNotificationWorkerThread, 0, sizeof(gNotificationWorkerThread));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief StartNotificationProcessWaitTimer
 * @param timeInSeconds
 * @return
 */
static BOOL StartNotificationProcessWaitTimer(UINT16 timeInSeconds)
{
    TIMER_INFO_t timerInfo;

    timerInfo.data = 0;
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(timeInSeconds);
    timerInfo.funcPtr = EnablePushNotificationProcess;

    if (gProcessWaitTimerHandle == INVALID_TIMER_HANDLE)
    {
        // start a new timer
        return StartTimer(timerInfo, &gProcessWaitTimerHandle);
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief EnablePushNotificationProcess
 * @param data
 */
static void EnablePushNotificationProcess(UINT32 data)
{
    // delete timer as it is required to call only once
    if (gProcessWaitTimerHandle != INVALID_TIMER_HANDLE)
    {
        DeleteTimer(&gProcessWaitTimerHandle);
    }

    DPRINT(PUSH_NOTIFY, "start push notification: wait timer expired");

    // enable notification process flag
    gStartPushNotification = TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief InitNotificationSessionParam
 */
static void InitNotificationSessionParam(void)
{
    UINT8 deviceIndex;

    MUTEX_INIT(gSessionParamMutex, NULL);

    // enable push notificationfor all devices present in the configuration
    for (deviceIndex = 0; deviceIndex < FCM_PUSH_NOTIFY_DEVICES_MAX; deviceIndex++)
    {
        // read push notification table from configuration
        ReadSingleFcmPushNotificationConfig(deviceIndex, &gNotificationSessionParam[deviceIndex].deviceConfig);

        if ('\0' != gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken[0])
        {
            DPRINT(PUSH_NOTIFY, "push notification device: [id=%d], [username=%s], [modelName=%s], [fcmToken=%s]",
                   deviceIndex,
                   gNotificationSessionParam[deviceIndex].deviceConfig.username,
                   gNotificationSessionParam[deviceIndex].deviceConfig.deviceModelName,
                   gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken);
        }

        // check how many devices are added in system
        if ('\0' == gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken[0])
        {
            // disable push notification
            gNotificationSessionParam[deviceIndex].notificationEnabled = FALSE;
            gNotificationSessionParam[deviceIndex].sessionIndex = INVALID_SESSION_INDEX;
            gNotificationSessionParam[deviceIndex].deviceExpiryTimestamp = 0;
        }
        else
        {
            // enable push notification
            gNotificationSessionParam[deviceIndex].notificationEnabled = TRUE;
            gNotificationSessionParam[deviceIndex].sessionIndex = INVALID_SESSION_INDEX;
            gNotificationSessionParam[deviceIndex].deviceExpiryTimestamp = GetDeviceExpiryTime();
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief DefaultNotificationSessionParam
 */
static void DefaultNotificationSessionParam(void)
{
    UINT8 deviceIndex;

    // enable push notificationfor all devices present in the configuration
    for (deviceIndex = 0; deviceIndex < FCM_PUSH_NOTIFY_DEVICES_MAX; deviceIndex++)
    {
        // default RAM copy of configuration
        memset(&gNotificationSessionParam[deviceIndex].deviceConfig, 0, sizeof(FCM_PUSH_NOTIFY_CONFIG_t));

        // default session parameters
        gNotificationSessionParam[deviceIndex].notificationEnabled = FALSE;
        gNotificationSessionParam[deviceIndex].sessionIndex = INVALID_SESSION_INDEX;
        gNotificationSessionParam[deviceIndex].deviceExpiryTimestamp = 0;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief DefaultNotificationSingleSessionParam
 * @param deviceIndex
 */
static void DefaultNotificationSingleSessionParam(UINT8 deviceIndex)
{
    if (deviceIndex >= FCM_PUSH_NOTIFY_DEVICES_MAX)
    {
        return;
    }

    // default RAM copy of configuration
    memset(&gNotificationSessionParam[deviceIndex].deviceConfig, 0, sizeof(FCM_PUSH_NOTIFY_CONFIG_t));

    // default session parameters
    gNotificationSessionParam[deviceIndex].notificationEnabled = FALSE;
    gNotificationSessionParam[deviceIndex].sessionIndex = INVALID_SESSION_INDEX;
    gNotificationSessionParam[deviceIndex].deviceExpiryTimestamp = 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief UpdateSessionParameters
 * @param deviceIndex
 * @param sessionIndex
 * @param fcmToken
 * @param enablePushNotification
 */
static void UpdateSessionParameters(UINT8 deviceIndex, UINT8 sessionIndex, BOOL enablePushNotification)
{
    // store/update data
    gNotificationSessionParam[deviceIndex].sessionIndex = sessionIndex;
    gNotificationSessionParam[deviceIndex].notificationEnabled = enablePushNotification;

    if (TRUE == enablePushNotification)
    {
        gNotificationSessionParam[deviceIndex].deviceExpiryTimestamp = GetDeviceExpiryTime();
    }
    else
    {
        // expiry timer not applicable if notification are off (user is online)
        gNotificationSessionParam[deviceIndex].deviceExpiryTimestamp = 0;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief RemoveAllDeviceOfUserForNotification
 * @param username
 */
static void RemoveAllDeviceOfUserForNotification(CHARPTR username)
{
    // search for all devices which matched with username
    UINT8 deviceIndex;

    DPRINT(PUSH_NOTIFY, "remove all device of user: [username=%s]", username);

    MUTEX_LOCK(gSessionParamMutex);

    // enable push notificationfor all devices present in the configuration
    for (deviceIndex = 0; deviceIndex < FCM_PUSH_NOTIFY_DEVICES_MAX; deviceIndex++)
    {
        if (strcmp(gNotificationSessionParam[deviceIndex].deviceConfig.username, username) != STATUS_OK)
        {
            continue;
        }

        DPRINT(PUSH_NOTIFY, "remove device: username matched: [username=%s], [modelName=%s], [fcmToken=%s]",
               gNotificationSessionParam[deviceIndex].deviceConfig.username,
               gNotificationSessionParam[deviceIndex].deviceConfig.deviceModelName,
               gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken);

        // default configuration & session parameters
        RemoveDeviceForNotification(deviceIndex);
    }

    MUTEX_UNLOCK(gSessionParamMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief EnableDevicePushNotification
 * @param sessionIndex
 * @param fcmToken
 * @param deviceModelname
 * @return
 */
static NET_CMD_STATUS_e EnableDevicePushNotification(UINT8 sessionIndex, CHARPTR fcmToken, CHARPTR deviceModelname)
{
    USER_ACCOUNT_CONFIG_t userAccountConfig;
    NET_CMD_STATUS_e ret = CMD_SUCCESS;

    MUTEX_LOCK(gSessionParamMutex);

    // check if device is already added in the list or not
    if (GetDeviceIndexFromFcmToken(fcmToken) < (UINT8)FCM_PUSH_NOTIFY_DEVICES_MAX)
    {
        MUTEX_UNLOCK(gSessionParamMutex);
        return CMD_SUCCESS;
    }

    // get user index from the session index & read configuration
    ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);

    // if already 10 devices are present
    if (GetTotalNotificationDeviceCount() >= (UINT8)FCM_PUSH_NOTIFY_DEVICES_MAX)
    {
        MUTEX_UNLOCK(gSessionParamMutex);

        EPRINT(PUSH_NOTIFY, "fail to add device: enable notification: max device limit exceeded: [username=%s], [modelName=%s], [fcmToken=%s]",
               userAccountConfig.username, deviceModelname, fcmToken);

        return CMD_MAX_USER_SESSION;
    }

    // save new device to configuration
    ret = SaveNewDeviceForNotification(sessionIndex, userAccountConfig.username, fcmToken, deviceModelname);

    if (CMD_SUCCESS == ret)
    {
        DPRINT(PUSH_NOTIFY, "new device added: enable notification: [username=%s], [modelName=%s], [fcmToken=%s]",
               userAccountConfig.username, deviceModelname, fcmToken);
    }
    else
    {
        EPRINT(PUSH_NOTIFY, "fail to add device: enable notification: [error=%d], [username=%s], [modelName=%s], [fcmToken=%s]",
               ret, userAccountConfig.username, deviceModelname, fcmToken);
    }

    MUTEX_UNLOCK(gSessionParamMutex);

    return ret;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief DisableDevicePushNotification
 * @param fcmToken
 * @return
 */
static NET_CMD_STATUS_e DisableDevicePushNotification(CHARPTR fcmToken)
{
    UINT8 deviceIndex;

    MUTEX_LOCK(gSessionParamMutex);

    // get device index from fcm token
    deviceIndex = GetDeviceIndexFromFcmToken(fcmToken);

    if (deviceIndex < FCM_PUSH_NOTIFY_DEVICES_MAX)
    {
        DPRINT(PUSH_NOTIFY, "remove device: disable notification: [username=%s], [modelName=%s], [fcmToken=%s]",
               gNotificationSessionParam[deviceIndex].deviceConfig.username,
               gNotificationSessionParam[deviceIndex].deviceConfig.deviceModelName,
               gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken);

        // remove device from configuration
        RemoveDeviceForNotification(deviceIndex);
    }

    MUTEX_UNLOCK(gSessionParamMutex);

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SaveNewDeviceForNotification
 * @param username
 * @param fcmToken
 * @param deviceModelname
 * @return
 */
static NET_CMD_STATUS_e SaveNewDeviceForNotification(UINT8 sessionIndex, CHARPTR username, CHARPTR fcmToken, CHARPTR deviceModelname)
{
    UINT8 deviceIndex;

    // get free index to save configuration
    deviceIndex = GetFreeDeviceIndex();

    if (deviceIndex >= FCM_PUSH_NOTIFY_DEVICES_MAX)
    {
        EPRINT(PUSH_NOTIFY, "fail to enable notification: no free device index found: [username=%s], [modelName=%s]",
               username, deviceModelname);

        return CMD_MAX_USER_SESSION;
    }

    // copy device information
    snprintf(gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken,
             sizeof(gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken), "%s", fcmToken);

    snprintf(gNotificationSessionParam[deviceIndex].deviceConfig.username,
             sizeof(gNotificationSessionParam[deviceIndex].deviceConfig.username), "%s", username);

    snprintf(gNotificationSessionParam[deviceIndex].deviceConfig.deviceModelName,
             sizeof(gNotificationSessionParam[deviceIndex].deviceConfig.deviceModelName), "%s", deviceModelname);

    // write configuration to file
    WriteSingleFcmPushNotificationConfig(deviceIndex, &(gNotificationSessionParam[deviceIndex].deviceConfig));

    // update session param
    UpdateSessionParameters(deviceIndex, sessionIndex, FALSE);

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief RemoveDeviceForNotification
 * @param deviceIndex
 * @return
 */
static NET_CMD_STATUS_e RemoveDeviceForNotification(UINT8 deviceIndex)
{
    // reset the session parameters
    DefaultNotificationSingleSessionParam(deviceIndex);

    // default entry in the configuration
    DfltSinglePushNotificationConfig(deviceIndex);

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetTotalNotificationDeviceCount
 * @return
 */
static UINT8 GetTotalNotificationDeviceCount(void)
{
    UINT8 deviceIndex;
    UINT8 activeDeviceCount = 0;

    for (deviceIndex = 0; deviceIndex < FCM_PUSH_NOTIFY_DEVICES_MAX; deviceIndex++)
    {
        if ('\0' != gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken[0])
        {
            activeDeviceCount++;
        }
    }

    return activeDeviceCount;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetFreeDeviceIndex
 * @return
 */
static UINT8 GetFreeDeviceIndex(void)
{
    UINT8 deviceIndex;

    // check for blank device fcm token
    for (deviceIndex = 0; deviceIndex < FCM_PUSH_NOTIFY_DEVICES_MAX; deviceIndex++)
    {
        // check for fcm token
        if ('\0' == gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken[0])
        {
            return deviceIndex;
        }
    }

    return FCM_PUSH_NOTIFY_DEVICES_MAX;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetDeviceIndexFromSessionIndex
 * @param sessionIndex
 * @return
 */
static UINT8 GetDeviceIndexFromSessionIndex(UINT8 sessionIndex)
{
    UINT8 deviceIndex;

    // find the given session index
    for (deviceIndex = 0; deviceIndex < FCM_PUSH_NOTIFY_DEVICES_MAX; deviceIndex++)
    {
        if (sessionIndex == gNotificationSessionParam[deviceIndex].sessionIndex)
        {
            return deviceIndex;
        }
    }

    return FCM_PUSH_NOTIFY_DEVICES_MAX;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetDeviceIndexFromFcmToken
 * @param fcmToken
 * @return
 */
static UINT8 GetDeviceIndexFromFcmToken(CHARPTR fcmToken)
{
    UINT8 deviceIndex;

    // compare device fcm token
    for (deviceIndex = 0; deviceIndex < FCM_PUSH_NOTIFY_DEVICES_MAX; deviceIndex++)
    {
        if (0 == strcmp(gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken, fcmToken))
        {
            return deviceIndex;
        }
    }

    return FCM_PUSH_NOTIFY_DEVICES_MAX;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetDeviceExpiryTime
 * @return
 */
static UINT64 GetDeviceExpiryTime(void)
{
    time_t currentTimeSinceEpoch;
    UINT64 expiryTimeSinceEpoch;

    GetLocalTimeInSec(&currentTimeSinceEpoch);

    expiryTimeSinceEpoch = currentTimeSinceEpoch + (DEVICE_INACTIVITY_TIMER_DAYS * SEC_IN_ONE_DAY);

    return expiryTimeSinceEpoch;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief PerformPushNotificationRequest
 * @param pQueueEntry
 */
static void PerformPushNotificationRequest(PUSH_NOTIFICATION_QUEUE_ENTRY_t *pQueueEntry)
{
    NOTIFICATION_PAYLOAD_PARAM_t notificationPayload;
    FCM_ERROR_CODE_e fcmResponse = FCM_NO_ERROR;
    CHAR deviceTokenList[FCM_PUSH_NOTIFY_DEVICES_MAX][FCM_TOKEN_LENGTH_MAX];
    UINT8 deviceTokenCount = 0;
    UINT8 successCount = 0;
    UINT8 count;
    BOOL retryNotification = FALSE;
    BOOL fatalError = FALSE;

    if (pQueueEntry == NULL)
    {
        EPRINT(PUSH_NOTIFY, "fail to perform push notification: invalid queue entry");
        return;
    }

    // Prepare list of device tokens to send push notification
    PrepareDeviceTokenList(deviceTokenList, &deviceTokenCount);

    if (0 == deviceTokenCount)
    {
        EPRINT(PUSH_NOTIFY, "fail to prepare fcm device token list");
        return;
    }

    DPRINT(PUSH_NOTIFY, "push notification request: [title=%s], [body=%s], [deviceCount=%d]",
           pQueueEntry->title, pQueueEntry->body, deviceTokenCount);

    //----------------------------------------------------------------
    // Prepare payload data for notification
    //----------------------------------------------------------------
    if (TRUE != PrepareFcmNotificationData(pQueueEntry, &notificationPayload))
    {
        EPRINT(PUSH_NOTIFY, "fail to prepare notification paylaod data");
        return;
    }

    //----------------------------------------------------------------
    // If we don't have valid access token, generate new one
    //----------------------------------------------------------------
    if (('\0' == gOauthTokenParam.accessToken[0]) || (0 == gOauthTokenParam.expiresInSeconds))
    {
        DPRINT(PUSH_NOTIFY, "getting initial oauth access token");

        if (TRUE != RefreshOauthAccessToken(&gOauthTokenParam))
        {
            EPRINT(PUSH_NOTIFY, "fail to get initial oauth access token");
            return;
        }
    }

    //----------------------------------------------------------------
    // Send push notification to each of the device token
    //----------------------------------------------------------------
    for (count = 0; count < deviceTokenCount; count++)
    {
        // push notification request
        if (TRUE != PerformPushNotification(deviceTokenList[count], gOauthTokenParam.accessToken, &notificationPayload, &fcmResponse))
        {
            EPRINT(PUSH_NOTIFY, "fail to send push notification: [tokenIndex=%d]", (count + 1));
            continue;
        }

        // Check the push notification response
        if (fcmResponse == FCM_NO_ERROR)
        {
            successCount++;
            continue;
        }

        EPRINT(PUSH_NOTIFY, "push notification failure response: [tokenIndex=%d], [httpResponseCode=%d]",
               (count + 1), fcmResponse);

        //----------------------------------------------------------------
        // Process fcm error responses
        //----------------------------------------------------------------
        ProcessFcmErrorResponse(fcmResponse, deviceTokenList[count], &fatalError, &retryNotification);

        // In case of fatal error, ignore other devices for current push notification
        if (TRUE == fatalError)
        {
            DPRINT(PUSH_NOTIFY, "ignore ramaining devices for push notification");
            break;
        }

        //----------------------------------------------------------------
        // Retry previous push notification
        //----------------------------------------------------------------
        if (TRUE == retryNotification)
        {
            // Clear flag to be use for next device
            retryNotification = FALSE;

            if (TRUE != PerformPushNotification(deviceTokenList[count], gOauthTokenParam.accessToken, &notificationPayload, &fcmResponse))
            {
                continue;
            }

            if (fcmResponse == FCM_NO_ERROR)
            {
                successCount++;
            }
        }
    }

    DPRINT(PUSH_NOTIFY, "push notification status: [total=%d], [succeed=%d]", deviceTokenCount, successCount);
}

/**
 * @brief ProcessFcmErrorResponse
 * @param fcmResponse
 * @param deviceToken
 * @param pFatalError
 * @param pRetryNotification
 */
static void ProcessFcmErrorResponse(FCM_ERROR_CODE_e fcmResponse, const CHARPTR deviceToken,
                                    BOOL *pFatalError, BOOL *pRetryNotification)
{
    *pFatalError = FALSE;
    *pRetryNotification = FALSE;

    switch (fcmResponse)
    {
        // Permanent error, no need to continue for remaining devices
        case FCM_ERROR_INVALID_ARGUMENT:
        case FCM_ERROR_SENDER_ID_MISMATCH:
        case FCM_ERROR_QUOTA_EXCEEDED:
        case FCM_RESPONSE_ERR_INTERNAL_SERVER_ERR:
        case FCM_RESPONSE_ERR_SERVER_UNAVAILABLE:
        {
            *pFatalError = TRUE;
        }
        break;

        // Remove the device as token is no more registered with fcm server
        case FCM_ERROR_UNREGISTERED:
        {
            UINT8 deviceIndex;

            EPRINT(PUSH_NOTIFY, "device not registered or invalid: [fcmToken=%s]", deviceToken);

            MUTEX_LOCK(gSessionParamMutex);

            deviceIndex = GetDeviceIndexFromFcmToken(deviceToken);

            if (deviceIndex < FCM_PUSH_NOTIFY_DEVICES_MAX)
            {
                DPRINT(PUSH_NOTIFY, "remove device: fcm server declared device not registered or invalid: [username=%s], [modelName=%s], [fcmToken=%s]",
                       gNotificationSessionParam[deviceIndex].deviceConfig.username,
                       gNotificationSessionParam[deviceIndex].deviceConfig.deviceModelName,
                       gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken);

                // remove device
                RemoveDeviceForNotification(deviceIndex);
            }

            MUTEX_UNLOCK(gSessionParamMutex);
        }
        break;

        // Authentication error
        case FCM_ERROR_AUTHENTICATION:
        {
            DPRINT(PUSH_NOTIFY, "refreshing oauth access token");

            // Need to retry with new access token
            if (FALSE == RefreshOauthAccessToken(&gOauthTokenParam))
            {
                // No need process futher as we failed to refresh oauth access token
                *pFatalError = TRUE;
            }
            else
            {
                *pRetryNotification = TRUE;
            }
        }
        break;

        default:
            break;
    }
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief Base64URL encode function
 *
 * @param input Input data to encode
 * @param length Length of input data
 * @return Encoded Base64URL string
 */
static CHAR *Base64UrlEncode(const UINT8 *input, size_t len)
{
    // Base64url encode map
    const CHAR base64UrlChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    // Calculate the output length
    size_t outputLen = 4 * ((len + 2) / 3);

    // Allocate memory for the encoded string
    CHAR *encoded = (CHAR *)malloc(outputLen + 1);

    if (!encoded)
    {
        // Memory allocation failed
        return NULL;
    }

    // Encode input to base64url
    size_t i, j, k;

    for (i = 0, j = 0; i < len;)
    {
        UINT32 octet_a = i < len ? input[i++] : 0;
        UINT32 octet_b = i < len ? input[i++] : 0;
        UINT32 octet_c = i < len ? input[i++] : 0;

        UINT32 triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded[j++] = base64UrlChars[(triple >> 3 * 6) & 0x3F];
        encoded[j++] = base64UrlChars[(triple >> 2 * 6) & 0x3F];
        encoded[j++] = base64UrlChars[(triple >> 1 * 6) & 0x3F];
        encoded[j++] = base64UrlChars[(triple >> 0 * 6) & 0x3F];
    }

    // Adjust for padding
    for (k = 0; k < (3 - len % 3) % 3; k++)
    {
        encoded[outputLen - 1 - k] = '\0';
    }

    // Null-terminate the string
    encoded[outputLen - (3 - len % 3) % 3] = '\0';

    return encoded;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief Sets options for a curl easy handle
 * @param handle
 */
static void SetCurlDefaultOptions(CURL *handle)
{
    // enable verbose output for debugging
    // curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
    // curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, curl_debug_cb);

    // user agent
    curl_easy_setopt(handle, CURLOPT_USERAGENT, curl_version());

    // tcp connection timeout
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, CURLOPT_TCP_CONN_TIMEOUT_SECONDS);

    // complete operation within given seconds
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, CURLOPT_OPERATION_TIMEOUT_SECONDS);

    // option to inform curl to do not use signal
    curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);

    // low speed limit & time
    curl_easy_setopt(handle, CURLOPT_LOW_SPEED_LIMIT, CULROPT_LOW_SPEED_LIMIT);
    curl_easy_setopt(handle, CURLOPT_LOW_SPEED_TIME, CULROPT_LOW_SPEED_TIME);

    // inform curl to stop all progress meter
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);

    // disable verification of peer ssl certificate
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CurlWiteDataCallback
 * @param pContents
 * @param size
 * @param nmemb
 * @param userData
 * @return
 */
static size_t CurlWiteDataCallback(void *pContents, size_t size, size_t nmemb, void *userData)
{
    // calculate buffer size
    size_t realsize = size * nmemb;

    if (NULL == userData || NULL == pContents)
    {
        /* NOTE: If Other than real size send then curl will create signal
         * It makes Existing Connection to Pause. So Thread will not free and Connection Pause Permenent untill Reboot.
         * So, even for Failed case we are not return -1
         */
        return realsize;
    }

    // cast pointer to fetch struct
    CURL_RESPONSE_PAYLOAD_t *pHttpRespPayload = (CURL_RESPONSE_PAYLOAD_t *)userData;

    // expand buffer. NOTE: Extra byte for Null termination
    CHAR *pTemp = (CHAR *) realloc(pHttpRespPayload->pPayloadString, pHttpRespPayload->payloadSize + realsize + 1);

    // Check Memory assignment success
    if (NULL == pTemp)
    {
        // return error because it is memory issue.
        // callback function called from curl library. Do mot return own error code
        return 0;
    }

    // assign rellocated memory
    pHttpRespPayload->pPayloadString = pTemp;

    // copy contents to buffer
    memcpy(&(pHttpRespPayload->pPayloadString[pHttpRespPayload->payloadSize]), pContents, realsize);

    // set new buffer size
    pHttpRespPayload->payloadSize += realsize;

    // ensure null termination
    pHttpRespPayload->pPayloadString[pHttpRespPayload->payloadSize] = 0;

    // return size
    return realsize;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief Decrypt the service account private key
 * @return TRUE on success, FALSE otherwise
 */
static BOOL DecryptServiceAccountPkey(void)
{
    CHAR *encryptedPkey;
    UINT8 *decryptedPkey;
    UINT32 encryptedKeyLen = 0;
    UINT32 decryptedKeyLen = 0;

    // First decode the base64 private key
    encryptedPkey = DecodeBase64(PRIVATE_KEY_AES256_BASE64,
                                 strlen(PRIVATE_KEY_AES256_BASE64),
                                 &encryptedKeyLen);

    if (NULL == encryptedPkey)
    {
        EPRINT(PUSH_NOTIFY, "fail to decode base64 private key");
        return FALSE;
    }

    // Decrypt the private key
    decryptedPkey = DecryptAes256((UINT8 *)encryptedPkey, encryptedKeyLen, (CHAR *)gPkeyDecryptPassword, (CHAR *)gPkeyDecryptSalt,
                                  &decryptedKeyLen);

    if (NULL == decryptedPkey)
    {
        free(encryptedPkey);
        EPRINT(PUSH_NOTIFY, "fail to decrypt aes256 encrypted private key");
        return FALSE;
    }

    // save the decrypted key
    snprintf(gServiceAccountPkey, sizeof(gServiceAccountPkey), "%s", decryptedPkey);

    free(encryptedPkey);
    free(decryptedPkey);

    return TRUE;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief Generates JWT Token using service account data
 * @param jwtToken
 * @return TRUE on success, FALSE otherwise
 */
static BOOL GenerateJwtToken(CHARPTR jwtToken)
{
    JWT_DATA_t jwtData;
    CHAR jsonHeader[256];
    CHAR jsonClaims[512];
    CHAR *encodedHeader;
    CHAR *encodedClaims;
    CHAR jsonPayloadFinal[JWT_JSON_PAYLOAD_SIZE];

    BOOL result = FALSE;
    BIO *bio = NULL;
    EVP_PKEY *privateKeyCtx = NULL;
    EVP_MD_CTX *ctx = NULL;
    UINT8 *signature = NULL;
    size_t signatureLen = 0;
    CHAR *b64Signature = NULL;

    //----------------------------------------------------------------
    // Create jwt headers & payloads
    //----------------------------------------------------------------

    // Prepare jwt headers fields
    snprintf(jwtData.header.algorithm, sizeof(jwtData.header.algorithm), "%s", JWT_SIGNING_ALGORITHM);
    snprintf(jwtData.header.type, sizeof(jwtData.header.type), "%s", JWT_TYPE);

    // Prepare jwt payload fields
    snprintf(jwtData.payload.issuer, sizeof(jwtData.payload.issuer), "%s", JWT_ISSUER);
    snprintf(jwtData.payload.scope, sizeof(jwtData.payload.issuer), "%s", JWT_SCOPE);
    snprintf(jwtData.payload.audience, sizeof(jwtData.payload.audience), "%s", JWT_AUDIENCE);

    // Token valid for 1 hour
    jwtData.payload.now = time(NULL);
    jwtData.payload.exp = jwtData.payload.now + 3600;

    //----------------------------------------------------------------
    // Encode jwt headers & payloads to base64url
    //----------------------------------------------------------------

    // JWT JSON Header
    snprintf(jsonHeader, sizeof(jsonHeader),
             "{\"alg\":\"%s\",\"typ\":\"%s\"}",
             jwtData.header.algorithm, jwtData.header.type);

    // Encode to base64
    encodedHeader = Base64UrlEncode((UINT8 *)jsonHeader, strlen(jsonHeader));

    // JWT JSON Claims
    snprintf(jsonClaims, sizeof(jsonClaims),
             "{\"iss\":\"%s\",\"scope\":\"%s\",\"aud\":\"%s\",\"iat\":%ld,\"exp\":%ld}",
             jwtData.payload.issuer, jwtData.payload.scope, jwtData.payload.audience,
             jwtData.payload.now, jwtData.payload.exp);

    // Encode to base64
    encodedClaims = Base64UrlEncode((UINT8 *)jsonClaims, strlen(jsonClaims));

    // Concatenate Header and Payload
    snprintf(jsonPayloadFinal, JWT_JSON_PAYLOAD_SIZE, "%s.%s", encodedHeader, encodedClaims);

    FREE_MEMORY(encodedHeader);
    FREE_MEMORY(encodedClaims);

    //----------------------------------------------------------------
    // Add digital signature
    //----------------------------------------------------------------
    do
    {
        // Load private key from string
        bio = BIO_new_mem_buf((void *)gServiceAccountPkey, -1);

        if (!bio)
        {
            EPRINT(PUSH_NOTIFY, "fail to create BIO for private key");
            break;
        }

        // Read private key from the BIO into an EVP_PKEY object
        privateKeyCtx = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);

        if (!privateKeyCtx)
        {
            EPRINT(PUSH_NOTIFY, "fail to load private key");
            break;
        }

        // An EVP_MD_CTX context is created for signing
        ctx = EVP_MD_CTX_create();

        if (!ctx)
        {
            EPRINT(PUSH_NOTIFY, "fail to create EVP_MD_CTX context");
            break;
        }

        // Initialize context for signing with the SHA-256 algorithm and the private key
        if (EVP_DigestSignInit(ctx, NULL, EVP_sha256(), NULL, privateKeyCtx) != 1)
        {
            EPRINT(PUSH_NOTIFY, "fail to initialize digest sign context");
            break;
        }

        // Feed data into the signing context
        if (EVP_DigestSignUpdate(ctx, jsonPayloadFinal, strlen(jsonPayloadFinal)) != 1)
        {
            EPRINT(PUSH_NOTIFY, "fail to updated digest sign context");
            break;
        }

        // Calculate final signature length
        if (EVP_DigestSignFinal(ctx, NULL, &signatureLen) != 1)
        {
            EPRINT(PUSH_NOTIFY, "fail to finalize digest sign context");
            break;
        }

        // Allocate memory for the signature
        signature = malloc(signatureLen);

        if (!signature)
        {
            EPRINT(PUSH_NOTIFY, "fail to allocate memory for digital signature");
            break;
        }

        // Generate final signature
        if (EVP_DigestSignFinal(ctx, signature, &signatureLen) != 1)
        {
            EPRINT(PUSH_NOTIFY, "fail to finalize digest sign context-2");
            break;
        }

        // Base64 url-safe encoded signature
        b64Signature = Base64UrlEncode(signature, signatureLen);

        if (NULL == b64Signature)
        {
            EPRINT(PUSH_NOTIFY, "fail to encode signature to base64url");
            break;
        }

        //----------------------------------------------------------------
        // Copy the final generated JWT token
        //----------------------------------------------------------------
        snprintf(jwtToken, JWT_TOKEN_LENGTH, "%s.%s", jsonPayloadFinal, b64Signature);

        result = TRUE;

    } while (0);

    // Memory cleanup
    if (bio) BIO_free(bio);
    if (privateKeyCtx) EVP_PKEY_free(privateKeyCtx);
    if (ctx) EVP_MD_CTX_destroy(ctx);
    FREE_MEMORY(signature);
    FREE_MEMORY(b64Signature);

    return result;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief Get new oauth access token with http request to google auth server
 * @param pJwtToken json web token to validate service account
 * @param pOauthTokenParam structure to store retrieved access token
 * @return
 */
static BOOL RefreshOauthAccessToken(OAUTH_TOKEN_PARAM_t *pOauthTokenParam)
{
    CHAR jwtToken[JWT_TOKEN_LENGTH];
    CURL_RESPONSE_PAYLOAD_t curlResponse = {0};

    // Generate JWT Token
    if (TRUE != GenerateJwtToken(jwtToken))
    {
        EPRINT(PUSH_NOTIFY, "fail to generate jwt token");

        return FALSE;
    }

    // Get oauth access token
    if (TRUE != OauthTokenHttpRequest(jwtToken, &curlResponse))
    {
        EPRINT(PUSH_NOTIFY, "fail to get oauth access token");

        // Free curl memory
        FREE_MEMORY(curlResponse.pPayloadString);

        return FALSE;
    }

    // Parse the http json response to get oauth token
    if (TRUE != OauthTokenPraseHttpResponse(&curlResponse, pOauthTokenParam))
    {
        EPRINT(PUSH_NOTIFY, "fail to parse oauth access token");

        // Free curl memory
        FREE_MEMORY(curlResponse.pPayloadString);

        return FALSE;
    }

    // Free curl memory
    FREE_MEMORY(curlResponse.pPayloadString);

    // Check the http response code
    if (curlResponse.responseCode != FCM_NO_ERROR)
    {
        EPRINT(PUSH_NOTIFY, "fail to get oauth access token: [httpResponseCode=%d]", curlResponse.responseCode);
        return FALSE;
    }

//    DPRINT(PUSH_NOTIFY, "oauth token refreshed: [token=%s], [expires_in=%d sec]",
//           pOauthTokenParam->accessToken, pOauthTokenParam->expiresInSeconds);

    DPRINT(PUSH_NOTIFY, "oauth access token refreshed successfully");

    return TRUE;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief Do http request to get oauth access token
 * @param pJwtToken jwt token
 * @param curlResponse json response from server
 * @return TRUE if token is generated successfully, FALSE otherwise
 */
static BOOL OauthTokenHttpRequest(const CHARPTR pJwtToken, CURL_RESPONSE_PAYLOAD_t *pCurlResponse)
{
    CURL *handle;
    CURLcode curlRet;
    CHAR buffer[4096];
    struct curl_slist *slist = NULL;

    //-----------------------------------------------------------------------
    // New curl session
    //-----------------------------------------------------------------------
    handle = curl_easy_init();

    if (NULL == handle)
    {
        EPRINT(PUSH_NOTIFY, "fail to allcoate curl session for oauth token request");
        return FALSE;
    }

    //-----------------------------------------------------------------------
    // curl options
    //-----------------------------------------------------------------------

    // default options
    SetCurlDefaultOptions(handle);

    // enable https
    curl_easy_setopt(handle, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);

    // set HTTP version HTTP/1.1
    curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_1_1);

    // set HTTP POST request
    curl_easy_setopt(handle, CURLOPT_POST, 1L);

    //-----------------------------------------------------------------------
    // curl http header & data
    //-----------------------------------------------------------------------

    // endpoint URL
    curl_easy_setopt(handle, CURLOPT_URL, OAUTH_TOKEN_REQUEST_ENDPOINT);

    // bind http headers to curl session
    slist = curl_slist_append(slist, OAUTH_TOKEN_REQ_CONTENT_TYPE);
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, slist);

    // bind http post fields to curl session
    snprintf(buffer, sizeof(buffer), OAUTH_TOKEN_REQ_POST_DATA, pJwtToken);

    // size of the data to copy from the buffer and send in the request
    curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, strlen(buffer));
    curl_easy_setopt(handle, CURLOPT_COPYPOSTFIELDS, buffer);

    //-----------------------------------------------------------------------
    // curl callback to get response data
    //-----------------------------------------------------------------------

    // reset response_string size to zero
    pCurlResponse->payloadSize = 0;

    // set callback function
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CurlWiteDataCallback);

    // pass userdata structure pointer
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)pCurlResponse);

    //-----------------------------------------------------------------------
    // perform HTTPS POST request
    //-----------------------------------------------------------------------
    do
    {
        // curl easy perform
        if (CURLE_OK != (curlRet = curl_easy_perform(handle)))
        {
            EPRINT(PUSH_NOTIFY, "fail to execute curl_easy_perform: [curlReturn=%d]", curlRet);
            break;
        }

        // verify HTTP response code
        long responseCode = 0;

        // last arg must be long ptr or ptr to char*
        if (CURLE_OK != curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &responseCode))
        {
            EPRINT(PUSH_NOTIFY, "fail to execute curl_easy_getinfo: [curlReturn=%d]", curlRet);
        }

        // save response code
        pCurlResponse->responseCode = responseCode;

    } while (0);

    // free slist
    if (NULL != slist)
    {
        curl_slist_free_all(slist);
    }

    // free curl handle
    curl_easy_cleanup(handle);

    if (CURLE_OK != curlRet)
    {
        return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief OauthTokenPraseHttpResponse
 * @param pCurlResponse
 * @param pOauthTokenParam
 * @return TRUE if token related data is parsed successfully, FALSE otherwise
 */
static BOOL OauthTokenPraseHttpResponse(CURL_RESPONSE_PAYLOAD_t *pCurlResponse, OAUTH_TOKEN_PARAM_t *pOauthTokenParam)
{
    json_error_t error;
    json_t *root;
    BOOL result = FALSE;

    // error if no payload is received
    if (NULL == pCurlResponse->pPayloadString)
    {
        EPRINT(PUSH_NOTIFY, "no json response present to parse oauth access token");
        return FALSE;
    }

    /*
     * Oauth2 access token sample json response payload:
     *
     * {"access_token":"ya29.c.c0AY_VpZiSSta9mTrck4c-J4gY1-cven7tKxG5MiaGxhSqC6VrKL6MxMRCTYaJlt2_NnFAzM6BnEa5tCy82t6BqL4npa4gjktmiRkTuz6rWYDbOQb95qZbESPxC4Gg-ELYlZszS_z-rM8_LcRxIOuf8HAzUbqCoSTi2g-
     * bt2ZpdnM5ZMUWh3QphbrArclkpiU_LHCYaU5nkwjxN96c7Q4OE-DFVJQTQ-lo8exgdTY5ztYFYp3XoDmVeWc6V9gP5wV9QRFKEPQDvu9DwHjQAeXixfY_vG4SWkgz8e4FBzY5WaVdpRPO3QC_4J1QoD48ITkVlxE5sUWDxRCbOMY7wZsTNzK6ltgaacvq
     * qXJ1TI2d6qftjFJIm3n3ZciOkQH387DwIect9u6v-1ojvByzjuS1hZxxJ1xpynhXWphq2YdJbIzmij0Qyzzs0YbU-pt48y7zvy_oF6g6lq5ZeaM7RzpcxaXUmUl87wJX9m5zMrV3IgV31swZbeucdgn3132q8snbZsZpwlqJy4d_0UY9VX41cUmhU3jMF
     * xJSj2zewnXkvbjoo2aluMolV651WOdYvla2zhWxeZooBdXMr7gS-5raRxqnjjzfWwobW0dm2UBh_cuF2uOpSjg4dMZezWzVW_qYk-hjpJ9yzdXpzXkUcUwFJWjwmI7pjXifxFqbw506vxYrxkOvUIrRv2h0vppdqbpzQlJ850ir8tdO4IoFaiBeOyMo
     * Xn0Ryn8vl6pQfZ2WBY3_R9MdfolJU3OX5h61YvQxWr-omIebsI316m9ys2SYxd-UgjBsp4F4IWjgkd-R9sb03bjXVc1B6_1xduY5bcdBF7ij-sSmhcv8-Us_80XUxQjp0Fv1hhJsfdr3Z9zmZYe2Qg3MRQQnrzygo9iBbrj99qkXFxcSsO5dcixynbfhI
     * JvxZnYabo5fIXapaim3gld11opX6851Z0bjagdi9Jev_71JkpX4WvsXBbXmaqOMjx4kQ8YXq3BemqZQZ4d6ZXcXMnbv6cQbU","expires_in":3599,"token_type":"Bearer"}
     */

    // Load the JSON response into a JSON object
    root = json_loads(pCurlResponse->pPayloadString, 0, &error);

    if (!root)
    {
        EPRINT(PUSH_NOTIFY, "fail to load oauth access token json response: [error=%s], [payload=%s]",
               error.text, pCurlResponse->pPayloadString);
        return FALSE;
    }

    do
    {
        // parse tag "access token"
        if (TRUE != JSON_GET_STRING(root, root, "access_token", pOauthTokenParam->accessToken,
                                    sizeof(pOauthTokenParam->accessToken)))
        {
            EPRINT(PUSH_NOTIFY, "fail to parse json tag 'access_token' in oauth json response: [payload=%s]",
                   pCurlResponse->pPayloadString);
            break;
        }

        // parse tag "expires_in"
        json_int_t expiresInSeconds = 0;
        if (TRUE != JSON_GET_INTEGER(root, root, "expires_in", &expiresInSeconds))
        {
            EPRINT(PUSH_NOTIFY, "fail to parse json tag 'expires_in' in oauth json response: [payload=%s]",
                   pCurlResponse->pPayloadString);

            break;
        }

        pOauthTokenParam->expiresInSeconds = expiresInSeconds;

        result = TRUE;

    } while(0);

    // Free the JSON object
    json_decref(root);

    return result;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief Prepare the list of device tokens to send a push notification
 * @param deviceTokenList list of device tokens
 * @param pTokenCount total count of device tokens filled in the list
 */
static void PrepareDeviceTokenList(CHAR deviceTokenList[][FCM_TOKEN_LENGTH_MAX], UINT8 *pTokenCount)
{
    UINT8 deviceIndex;
    UINT8 tokenIndex = 0;

    MUTEX_LOCK(gSessionParamMutex);

    for (deviceIndex = 0; deviceIndex < FCM_PUSH_NOTIFY_DEVICES_MAX; deviceIndex++)
    {
        if ('\0' != gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken[0])
        {
            // save valid fcm device tokens
            snprintf(deviceTokenList[tokenIndex++],
                     FCM_TOKEN_LENGTH_MAX,
                     "%s",
                     gNotificationSessionParam[deviceIndex].deviceConfig.deviceFcmToken);
        }
    }

    MUTEX_UNLOCK(gSessionParamMutex);

    // total device tokens in the list
    *pTokenCount = tokenIndex;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Prepare push notification payload data
 * @param pQueueEntry notitication queue single entry object
 * @param pPayloadParam notification payload parameters
 * @return TRUE on success, FALSE otherwise
 */
static BOOL PrepareFcmNotificationData(PUSH_NOTIFICATION_QUEUE_ENTRY_t *pQueueEntry, NOTIFICATION_PAYLOAD_PARAM_t *pPayloadParam)
{
    if ((pQueueEntry == NULL) || (pPayloadParam == NULL))
    {
        return FALSE;
    }

    // Notification title
    snprintf(pPayloadParam->title, sizeof(pPayloadParam->title), "%s", pQueueEntry->title);

    // Notification body
    snprintf(pPayloadParam->body, sizeof(pPayloadParam->body), "%s", pQueueEntry->body);

    // Notification data (date/time)
    snprintf(pPayloadParam->dateTime, sizeof(pPayloadParam->dateTime), "%s", pQueueEntry->dateTime);

    // Notification data (NVR MAC address)
    GetMacAddr(LAN1_PORT, pPayloadParam->lan1MacAddr);

    return TRUE;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief Prepare push notitification payload in json format
 * @param pPayloadParam payload title and body
 * @param deviceToken device token
 * @param pJsonPayloadStr pointer to json buffer
 * @return
 */
static BOOL PrepareFcmNotificationJsonPayload(NOTIFICATION_PAYLOAD_PARAM_t *pPayloadParam, CHARPTR deviceToken,
                                              CHARPTR pJsonPayloadStr)
{
    if ((NULL == pPayloadParam) || (NULL == pJsonPayloadStr))
    {
        return FALSE;
    }

    // prepare json formatted payload
    snprintf(pJsonPayloadStr, FCM_NOTIFY_PAYLOAD_LEN_MAX, PUSH_NOTIFICATION_PAYLOAD_DATA,
             deviceToken,
             pPayloadParam->title,
             pPayloadParam->body,
             pPayloadParam->dateTime,
             pPayloadParam->lan1MacAddr);

    return TRUE;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief PushNotificationHttpRequest
 * @param pJsonPayload
 * @param pOauthToken
 * @param pCurlResponse
 * @return
 */
static BOOL PushNotificationHttpRequest(CHARPTR pJsonPayload, CHARPTR pOauthToken, CURL_RESPONSE_PAYLOAD_t *pCurlResponse)
{
    CURL *handle;
    CURLcode curlRet;
    CHAR buffer[4096];
    struct curl_slist *slist = NULL;

    //-----------------------------------------------------------------------
    // New curl session
    //-----------------------------------------------------------------------
    handle = curl_easy_init();

    if (NULL == handle)
    {
        EPRINT(PUSH_NOTIFY, "fail to allcoate curl session for push notification");
        return FALSE;
    }

    //-----------------------------------------------------------------------
    // curl options
    //-----------------------------------------------------------------------

    // default options
    SetCurlDefaultOptions(handle);

    // enable https
    curl_easy_setopt(handle, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);

    // set HTTP version HTTP/1.1
    curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_1_1);

    // set HTTP POST request
    curl_easy_setopt(handle, CURLOPT_POST, 1L);

    //-----------------------------------------------------------------------
    // curl http header & data
    //-----------------------------------------------------------------------

    // endpoint URL
    curl_easy_setopt(handle, CURLOPT_URL, PUSH_NOTIFICATION_REQUEST_ENDPOINT);

    // authorization header
    snprintf(buffer, sizeof(buffer), PUSH_NOTIFICATION_REQ_AUTHORIZATION, pOauthToken);
    slist = curl_slist_append(slist, buffer);

    // content type header
    slist = curl_slist_append(slist, PUSH_NOTIFICATION_REQ_CONTENT_TYPE);

    // bind http header fields to curl session
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, slist);

    // bind http post fields to curl session
    snprintf(buffer, sizeof(buffer), "%s", pJsonPayload);

    // size of the data to copy from the buffer and send in the request
    curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, strlen(buffer));
    curl_easy_setopt(handle, CURLOPT_COPYPOSTFIELDS, buffer);

    //-----------------------------------------------------------------------
    // curl callback to get response data
    //-----------------------------------------------------------------------

    // reset response_string size to zero
    pCurlResponse->payloadSize = 0;

    // set callback function
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CurlWiteDataCallback);

    // pass userdata structure pointer
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)pCurlResponse);

    //-----------------------------------------------------------------------
    // perform HTTPS POST request
    //-----------------------------------------------------------------------
    do
    {
        // curl easy perform
        if (CURLE_OK != (curlRet = curl_easy_perform(handle)))
        {
            EPRINT(PUSH_NOTIFY, "fail to execute curl_easy_perform: [curlReturn=%d]", curlRet);
            break;
        }

        // verify HTTP response code
        long responseCode = 0;

        // last arg must be long ptr or ptr to char*
        if (CURLE_OK != curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &responseCode))
        {
            EPRINT(PUSH_NOTIFY, "fail to execute curl_easy_getinfo: [curlReturn=%d]", curlRet);
        }

        // save response code
        pCurlResponse->responseCode = responseCode;

    } while (0);

    // free slist
    if (NULL != slist)
    {
        curl_slist_free_all(slist);
    }

    // free curl handle
    curl_easy_cleanup(handle);

    if (CURLE_OK != curlRet)
    {
        return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief PushNotificationParseHttpResponse
 * @param pCurlResponse
 * @param pFcmResponse
 * @return
 */
static BOOL PushNotificationParseHttpResponse(CURL_RESPONSE_PAYLOAD_t *pCurlResponse, FCM_ERROR_CODE_e *pFcmResponse)
{
    // Parse json response if required to identify specific error on failure (code != 200 OK)
    *pFcmResponse = pCurlResponse->responseCode;

    return TRUE;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief Sends push notification to single device token
 * @param pDeviceToken FCM device token
 * @param pOauthToken Oauth access token
 * @param pPayload Notification payload data
 * @param pFcmResponse FCM response code
 * @return TRUE on success, FALSE otherwise
 */
static BOOL PerformPushNotification(CHARPTR pDeviceToken, CHARPTR pOauthToken,
                                 NOTIFICATION_PAYLOAD_PARAM_t *pPayload, FCM_ERROR_CODE_e *pFcmResponse)
{
    CURL_RESPONSE_PAYLOAD_t curlResponse = {0};
    CHAR jsonPayload[FCM_NOTIFY_PAYLOAD_LEN_MAX];

    // Prepare json payload
    if (TRUE != PrepareFcmNotificationJsonPayload(pPayload, pDeviceToken, jsonPayload))
    {
        return FALSE;
    }

    // Push notification http request
    if (TRUE != PushNotificationHttpRequest(jsonPayload, pOauthToken, &curlResponse))
    {
        EPRINT(PUSH_NOTIFY, "fail to send push notification request");

        // Free curl memory
        FREE_MEMORY(curlResponse.pPayloadString);

        return FALSE;
    }

    // Parse the http json response in case of failure
    if (TRUE != PushNotificationParseHttpResponse(&curlResponse, pFcmResponse))
    {
        EPRINT(PUSH_NOTIFY, "fail to parse push notification response");

        // Free curl memory
        FREE_MEMORY(curlResponse.pPayloadString);

        return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief JSON_GET_INTEGER
 * @param json_root
 * @param json_current
 * @param tag
 * @param value
 * @return
 */
static BOOL JSON_GET_INTEGER(json_t *json_root, json_t *json_current, CHAR *tag, json_int_t *value)
{
    json_t *json;
    json = json_object_get(json_current, tag);
    if (!json_is_integer(json))
    {
        return FALSE;
    }
    *value = json_integer_value(json);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief JSON_GET_STRING
 * @param json_root
 * @param json_current
 * @param tag
 * @param buff
 * @param buff_size
 * @return
 */
static BOOL JSON_GET_STRING(json_t *json_root, json_t *json_current, CHAR *tag, CHAR *buff, INT32 buff_size)
{
    json_t *json;
    json = json_object_get(json_current, tag);
    if (!json_is_string(json))
    {
        return FALSE;
    }
    snprintf(buff, buff_size, "%s", json_string_value(json));
    return TRUE;
}

//-------------------------------------------------------------------------------------------------

#if 0

/**
 * @brief JSON_GET_OBJECT
 * @param json_root
 * @param json_obj
 * @param tag
 * @return
 */
static BOOL JSON_GET_OBJECT(json_t *json_root, json_t **json_obj, CHAR *tag)
{
    *json_obj = json_object_get(json_root, tag);
    if (!json_is_object(*json_obj))
    {
        return FALSE;
    }
    return TRUE;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief JSON_GET_ARRAY
 * @param json_root
 * @param json_array
 * @param tag
 * @return
 */
static BOOL JSON_GET_ARRAY(json_t *json_root, json_t **json_array, CHAR *tag)
{
    *json_array = json_object_get(json_root, tag);
    if (!json_is_array(*json_array))
    {
        return FALSE;
    }
    return TRUE;
}

#endif

//#################################################################################################
// @END OF FILE
//#################################################################################################
