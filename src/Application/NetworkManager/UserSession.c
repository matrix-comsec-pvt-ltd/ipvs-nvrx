//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		UserSession.c
@brief      File maintains user sessions for all clients. Any Network module can request for user
            login/logout session.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "UserSession.h"
#include "DebugLog.h"
#include "Utils.h"
#include "ConfigApi.h"
#include "NetworkManager.h"
#include "NetworkCommand.h"
#include "ViewerUserSession.h"
#include "PlaybackMediaStreamer.h"
#include "SyncPlaybackMediaStreamer.h"
#include "InstantPlaybackMediaStreamer.h"
#include "CameraSearch.h"
#include "AdvanceCameraSearch.h"
#include "LiveMediaStreamer.h"
#include "FcmPushNotification.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define UPDATE_USER_SCRIPT              SCRIPTS_DIR_PATH "/updateUserList.sh"
#define USER_PASSWORD_RESET_FILE        CONFIG_DIR_PATH "/UserPasswordResetDur"
#define USER_SES_EXPIRE_FIRST_LOG_TIME  1
#define MAX_FIELD_VALUE_SIZE            25

#define PASS_HAS_UPPERCASE              (0x01)
#define PASS_HAS_LOWERCASE              (0x02)
#define PASS_HAS_NUMBER                 (0x04)
#define PASS_HAS_SPECIAL_CHAR           (0x08)
#define HIGH_PASSWORD_STRENGTH_VAL		(0x0F)
#define	MAX_POLL_TIMEOUT_DEVICE(t)      (t - 3) //Give ack after 5 sec if there is not any event

/* thread stack size */
#define USR_TIMER_THREAD_STACK_SZ       (1*MEGA_BYTE)

#define RETURN_IF_INVLD_SESSION_ID(sessionIdx) \
    if (sessionIdx >= MAX_NW_CLIENT) {EPRINT(SYS_LOG, "invld user session: [sessionIdx=%d]", sessionIdx); return;}

#define RETURN_VAL_IF_INVLD_SESSION_ID(sessionIdx, val) \
    if (sessionIdx >= MAX_NW_CLIENT) {EPRINT(SYS_LOG, "invld user session: [sessionIdx=%d]", sessionIdx); return val;}

#define RETURN_FALSE_IF_INVLD_SESSION_ID(sessionIdx)    RETURN_VAL_IF_INVLD_SESSION_ID(sessionIdx, FALSE)

#define RETURN_IF_INVLD_USER_ID(userIdx) \
    if (userIdx >= MAX_USER_ACCOUNT) {EPRINT(SYS_LOG, "invld user index: [userIdx=%d]", userIdx); return;}

#define RETURN_FALSE_IF_INVLD_USER_ID(userIdx) \
    if (userIdx >= MAX_USER_ACCOUNT) {EPRINT(SYS_LOG, "invld user index: [userIdx=%d]", userIdx); return FALSE;}

#define RETURN_VAL_IF_INVLD_PSWD_RST_SESSION_ID(sessionIdx, val) \
    if (sessionIdx >= PWD_RST_SESSION_MAX) {EPRINT(SYS_LOG, "invld pwd rst session: [sessionIdx=%d]", sessionIdx); return val;}

//#################################################################################################
// @DATA TYEPS
//#################################################################################################
typedef struct
{
    UINT8           invalidAttempts;
    UINT32          resetTimer;
    pthread_mutex_t passwordAttemptMutex;

}PASSWORD_ATTEMPTS_INFO_t;

typedef struct
{
    CHAR 	username[MAX_USER_ACCOUNT_USERNAME_WIDTH];
    CHAR 	password[MAX_USER_ACCOUNT_PASSWORD_WIDTH];
    UINT32	isPasswordResetReq;
    UINT32	timeSinceCreated;

}PASSWORD_RESET_INFO_t;

//#################################################################################################
// @EXTERN VARIABLES
//#################################################################################################
extern const CHARPTR headerReq[MAX_HEADER_REQ];

//#################################################################################################
// @GLOBAL VARIABLES
//#################################################################################################

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static USER_SESSION_INFO_t      userSessionInfo[MAX_NW_CLIENT];
static USER_PWD_RST_INFO_t      userPwdRstInfo[PWD_RST_SESSION_MAX];
static PASSWORD_ATTEMPTS_INFO_t passwordAttemptInfo[INVALID_ATTEMPT_TYPE_MAX][MAX_USER_ACCOUNT];

/* user block info */
static BOOL             userBlocked[MAX_USER_ACCOUNT];
static pthread_mutex_t  userBlockMutex = PTHREAD_MUTEX_INITIALIZER;

/* global variable for storing client events */
static EVENT_LOG_t      eventBuffer[MAX_EVENTS_IN_BUFFER];
static INT16            eventWriteIndex;
static pthread_mutex_t  eventMutex;

/* Local client login monitor timer */
static TIMER_HANDLE     hLocalClientReloginMonitorTmr = INVALID_TIMER_HANDLE;

/* max keep alive timer value as per client type */
static const UINT8 maxKeepAliveTime[NW_CLIENT_TYPE_MAX] =
{
    60, /* NW_CLIENT_TYPE_MOBILE */
    85, /* NW_CLIENT_TYPE_WEB */
    60, /* NW_CLIENT_TYPE_P2P_MOBILE */
    85  /* NW_CLIENT_TYPE_P2P_DESKTOP */
};

/* max polling interval as per client type */
static const UINT8 maxPollTime[NW_CLIENT_TYPE_MAX] =
{
    10, /* NW_CLIENT_TYPE_MOBILE */
    8,  /* NW_CLIENT_TYPE_WEB */
    10, /* NW_CLIENT_TYPE_P2P_MOBILE */
    8   /* NW_CLIENT_TYPE_P2P_DESKTOP */
};

/* Remote client name string */
static const CHAR *pClientNameStr[NW_CLIENT_TYPE_MAX] =
{
    MOBILE_CLIENT_EVT_STR,          /* NW_CLIENT_TYPE_MOBILE */
    DEVICE_CLIENT_EVT_STR,          /* NW_CLIENT_TYPE_WEB */
    P2P_MOBILE_CLIENT_EVT_STR,      /* NW_CLIENT_TYPE_P2P_MOBILE */
    P2P_DESKTOP_CLIENT_EVT_STR,     /* NW_CLIENT_TYPE_P2P_DESKTOP */
};

/* 8bytes are reserved for future use */
static const CHAR *pLoginReseverdBytes = "00000000";

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void initPasswordExpiryInfo(void);
//-------------------------------------------------------------------------------------------------
static VOIDPTR userTimerThread(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static void	resetAllViewerUserOnDayChange(void);
//-------------------------------------------------------------------------------------------------
static BOOL	deletPasswordExpiryInfo(UINT8 userId);
//-------------------------------------------------------------------------------------------------
static void generateLoginResponse(CHARPTR replyMsg, NET_CMD_STATUS_e cmdResponse, UINT32 sessionId, NW_CLIENT_TYPE_e clientType,
                                  UINT32 passLockDuration, UINT32 passResetInOneDay, USER_GROUP_e userGroup, UINT8 remainingLoginAttempts, UINT32 replyMsgLen);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e validateUser(UINT8 sessionIndex, CHARPTR userName, CHARPTR password, CHARPTR clientAddrStr, NW_CLIENT_TYPE_e clientType,
                                     UINT32PTR passLockDuration, UINT8PTR passResetInOneDay, USER_GROUP_e *userGroup, UINT8PTR remainingLoginAttempts);
//-------------------------------------------------------------------------------------------------
static INT32 assignSessionId(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e validateIpAddress(SOCK_ADDR_INFO_u *ipAddrInfo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e allocateUserSession(UINT8PTR pSessionIdx);
//-------------------------------------------------------------------------------------------------
static void deallocateUserSession(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
static void localClientLoginWaitTmrCb(UINT32 data);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e allocatePwdRstSession(UINT8PTR pSessionIdx);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e deriveClientType(CHARPTR smartCode, CLIENT_CB_TYPE_e clientCbType, NW_CLIENT_TYPE_e *pClientType);
//-------------------------------------------------------------------------------------------------
static void clearUserPasswordResetSession(UINT8 usrIndx);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief Init user session info, password expiry info, event buffer and start user timer thred
 */
void InitUserSession(void)
{
    UINT8 loop, index;

    /* init user session info */
    for (loop = 0; loop < (MAX_NW_CLIENT); loop++)
    {
        userSessionInfo[loop].isSessionActive = FALSE;
        userSessionInfo[loop].isPassResetReq = FALSE;
        userSessionInfo[loop].userIndex = MAX_USER_ACCOUNT;
        userSessionInfo[loop].clientType = NW_CLIENT_TYPE_MAX;
        userSessionInfo[loop].sessionId = -1;
        userSessionInfo[loop].pollFd = INVALID_CONNECTION;
        userSessionInfo[loop].eventReadIndex = 0;
        userSessionInfo[loop].pollTimerCnt = 0;
        userSessionInfo[loop].keepAliveTimerCnt = 0;
        userSessionInfo[loop].ipAddr[0] = '\0';
        userSessionInfo[loop].viewerUserCfgIndex = MAX_USER_ACCOUNT;
        userSessionInfo[loop].viewerUserSysTk = 0;
        userSessionInfo[loop].viewerSessMultiLogin = FALSE;
        userSessionInfo[loop].reLoginOnMultiLoginDisable = FALSE;
        MUTEX_INIT(userSessionInfo[loop].userSessionLock, NULL);
    }

    /* init user password reset session info */
    for (loop = 0; loop < PWD_RST_SESSION_MAX; loop++)
    {
        userPwdRstInfo[loop].isSessionActive = FALSE;
        userPwdRstInfo[loop].userIndex = MAX_USER_ACCOUNT;
        userPwdRstInfo[loop].clientType = NW_CLIENT_TYPE_MAX;
        userPwdRstInfo[loop].sessionId = -1;
        userPwdRstInfo[loop].sessionTmrCnt = 0;
        userPwdRstInfo[loop].ipAddr[0] = '\0';
        userPwdRstInfo[loop].otp6Digit = 0;
        MUTEX_INIT(userPwdRstInfo[loop].sessionLock, NULL);
    }

    /* Initialization of password expiry */
    initPasswordExpiryInfo();

    /* set all user to unblock */
    for(loop = 0; loop < MAX_USER_ACCOUNT; loop++)
    {
        userBlocked[loop] = FALSE;
        for (index = 0; index < INVALID_ATTEMPT_TYPE_MAX; index++)
        {
            passwordAttemptInfo[index][loop].invalidAttempts = 0;
            passwordAttemptInfo[index][loop].resetTimer = 0;
            MUTEX_INIT(passwordAttemptInfo[index][loop].passwordAttemptMutex, NULL);
        }
    }

    /* Init viewer user session info */
    InitViewerUserSession();

    /* Initialization of event related global variables */
    eventWriteIndex = 0;
    memset(eventBuffer, 0, sizeof(eventBuffer));
    MUTEX_INIT(eventMutex,NULL);

    /* Init user timer thread */
    if (FALSE == Utils_CreateThread(NULL, userTimerThread, NULL, DETACHED_THREAD, USR_TIMER_THREAD_STACK_SZ))
    {
        EPRINT(SYS_LOG, "failed to create viewer user timer thread");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process client login request, validate user, assign session and give login response
 * @param   loginReq
 * @param   clientAddr - Client socket address info
 * @param   connFd
 * @param   clientCbType
 * @param   sessionIndex
 * @return  TRUE on success; FALSE otherwise
 */
NET_CMD_STATUS_e ProcessClientLoginRequest(CHARPTR loginReq, SOCK_ADDR_INFO_u *clientAddr, UINT32 connFd, CLIENT_CB_TYPE_e clientCbType,
                                           NW_CLIENT_TYPE_e clientType, UINT8PTR sessionIdx)
{
    CHAR                fieldValue[MAX_FIELD_VALUE_SIZE] = "";
    CHAR                username[MAX_USER_ACCOUNT_USERNAME_WIDTH] = "";
    CHAR                password[MAX_USER_ACCOUNT_PASSWORD_WIDTH] = "";
    CHAR                fcmToken[FCM_TOKEN_LENGTH_MAX] = "";
    CHAR                ipAddrStr[IPV6_ADDR_LEN_MAX] = "";
    CHAR                loginResp[MAX_REPLY_SZ] = "";
    UINT8               passResetInOneDay = FALSE;
    UINT32              passLockDuration = 0;
    USER_GROUP_e        userGroup = MAX_USER_GROUP;
    NET_CMD_STATUS_e    status = CMD_PROCESS_ERROR;
    BOOL                fcmTokenFound = FALSE;
    UINT8               remainingLoginAttempts = 0;
    INT32 				sessionId;
    IP_NW_ADDR_u        ipAddrNw;

    do
    {
        /* Get IP address in network format from socket address */
        GetIpAddrNwFromSockAddr(clientAddr, &ipAddrNw);

        /* Get IP address in string format from socket address */
        GetIpAddrStrFromSockAddr(clientAddr, ipAddrStr, sizeof(ipAddrStr));

        /* Check if login request is from local client */
        if ((clientAddr->sockAddr.sa_family == AF_INET) && (ipAddrNw.ip4 == LOCAL_USER_IP))
        {
            *sessionIdx = USER_LOCAL;
        }
        else
        {
            /* check if ip is in block list or not */
            if ((status = validateIpAddress(clientAddr)) != CMD_SUCCESS)
            {
                break;
            }

            /* assign free user session */
            if ((status = allocateUserSession(sessionIdx)) != CMD_SUCCESS)
            {
                break;
            }
        }

        /* parse smart code for client type validation */
        if(ParseStr(&loginReq, FSP, fieldValue, MAX_FIELD_VALUE_SIZE) == FAIL)
        {
            deallocateUserSession(*sessionIdx);
            status = CMD_INVALID_SYNTAX;
            break;
        }

        /* validate client smart code  */
        if ((status = deriveClientType(fieldValue, clientCbType, &clientType)) != CMD_SUCCESS)
        {
            deallocateUserSession(*sessionIdx);
            break;
        }

        /* parse username */
        if(ParseStr(&loginReq, FSP, username, sizeof(username)) == FAIL)
        {
            deallocateUserSession(*sessionIdx);
            status = CMD_INVALID_SYNTAX;
            break;
        }

        /* parse password */
        if(ParseStr(&loginReq, FSP, password, sizeof(password)) == FAIL)
        {
            deallocateUserSession(*sessionIdx);
            status = CMD_INVALID_SYNTAX;
            break;
        }

        /* parse fcm token */
        if ((NW_CLIENT_TYPE_MOBILE == clientType) || (NW_CLIENT_TYPE_P2P_MOBILE == clientType))
        {
            if ((ParseStr(&loginReq, FSP, fcmToken, sizeof(fcmToken)) == SUCCESS) && ('\0' != fcmToken[0]))
            {
                fcmTokenFound = TRUE;
            }
            else
            {
                EPRINT(SYS_LOG, "fail to get device fcm token: [username=%s]", username);
            }
        }

        /* validate user information. If not valid then free user session */
        status = validateUser(*sessionIdx, username, password, ipAddrStr, clientType, &passLockDuration, &passResetInOneDay, &userGroup, &remainingLoginAttempts);

    }while(0);

    /* Update local session key and client type with valid value */
    sessionId = (*sessionIdx < MAX_NW_CLIENT) ? userSessionInfo[*sessionIdx].sessionId : -1;
    clientType = (clientType >= NW_CLIENT_TYPE_MAX) ? NW_CLIENT_TYPE_WEB : clientType;

    /* prepare login response as per status */
    generateLoginResponse(loginResp, status, sessionId, clientType, passLockDuration, passResetInOneDay, userGroup, remainingLoginAttempts, sizeof(loginResp));

    /* send login response to client */
    sendCmdCb[clientCbType](connFd, (UINT8PTR)loginResp, strlen(loginResp), MESSAGE_REPLY_TIMEOUT);

    /* consider all three status as login success */
    if ((status == CMD_SUCCESS) || (status == CMD_RESET_PASSWORD) || (status == CMD_PASSWORD_EXPIRE))
    {
        DPRINT(SYS_LOG, "user login success: [username=%s], [ip=%s], [sessionIdx=%d], [sessionKey=%d], [status=%d]",
               username, ipAddrStr, *sessionIdx, sessionId, status);

        /* if fcm token found in login request & login success, process device for push notification */
        if ((status == CMD_SUCCESS) && (TRUE == fcmTokenFound))
        {
            FcmPushNotifyMobileUserLogin(*sessionIdx, username, fcmToken);
        }
        status = CMD_SUCCESS;

        /* Check login wait timer for local client */
        if (*sessionIdx == USER_LOCAL)
        {
            /* Remove all session for local client. It may re-login due to application crash. */
            RemovePlayBackFromSessionId(*sessionIdx);
            RemoveSyncPlayBackFromSessionId(*sessionIdx, DISK_ACT_NORMAL);
            RemoveInstantPlayBackFromSessionId(*sessionIdx);
            StopCameraSearch(*sessionIdx);
            StopAdvanceCameraSearch(*sessionIdx);

			/* If timer is running then delete it */
            if (hLocalClientReloginMonitorTmr != INVALID_TIMER_HANDLE)
            {
                /* If local client is hanged and we have started login wait timer then stop it */
                DeleteTimer(&hLocalClientReloginMonitorTmr);
            }
        }
    }
    else
    {
        EPRINT(SYS_LOG, "user login failed: [username=%s], [ip=%s], [status=%d]", username, ipAddrStr, status);
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Logout user with given session index
 * @param   sessionIdx
 */
void UserLogout(UINT8 sessionIdx)
{
    UINT8					camIndex = 0;
    USER_ACCOUNT_CONFIG_t	userAccountConfig;
    VIDEO_TYPE_e			streamType;
    CLIENT_CB_TYPE_e        clientCbType;
    CHAR                    advncdetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    if (FALSE == IsUserSessionActive(sessionIdx))
    {
        EPRINT(SYS_LOG, "user session is already inactive: [sessionIdx=%d]", sessionIdx);
        return;
    }

    MUTEX_LOCK(userSessionInfo[sessionIdx].userSessionLock);
    /* Get client callback type from client type */
    if ((userSessionInfo[sessionIdx].clientType == NW_CLIENT_TYPE_P2P_MOBILE) ||
            (userSessionInfo[sessionIdx].clientType == NW_CLIENT_TYPE_P2P_DESKTOP))
    {
        clientCbType = CLIENT_CB_TYPE_P2P;
    }
    else
    {
        clientCbType = CLIENT_CB_TYPE_NATIVE;
    }

    /* Close polling socket */
    closeConnCb[clientCbType](&userSessionInfo[sessionIdx].pollFd);

    for(camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
    {
        for(streamType = MAIN_STREAM; streamType < MAX_STREAM; streamType++)
        {
            RemoveLiveMediaStream(camIndex, streamType, sessionIdx);
        }
    }

    /* Remove all active Playback session for current user/client */
    RemovePlayBackFromSessionId(sessionIdx);
    RemoveSyncPlayBackFromSessionId(sessionIdx, DISK_ACT_NORMAL);
    RemoveInstantPlayBackFromSessionId(sessionIdx);
    StopCameraSearch(sessionIdx);
    StopAdvanceCameraSearch(sessionIdx);

    /* in case of mobile client, update push notification status */
    if ((userSessionInfo[sessionIdx].clientType == NW_CLIENT_TYPE_MOBILE) || (userSessionInfo[sessionIdx].clientType == NW_CLIENT_TYPE_P2P_MOBILE))
    {
        FcmPushNotifyMobileUserLogout(sessionIdx);
    }

    /* Derive client type. If IP is local host then it is local client. If IP is not local host and
     * client type is web client than it is device client else it is mobile client */
    advncdetail[0] = '\0';
    if (strcmp(userSessionInfo[sessionIdx].ipAddr, LOCAL_CLIENT_IP_ADDR) == 0)
    {
        /* It is local client */
        snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, LOCAL_CLIENT_EVT_STR);
    }
    else if (userSessionInfo[sessionIdx].clientType < NW_CLIENT_TYPE_MAX)
    {
        /* It is remote client and also add its IP in event log */
        snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | %s",
                 pClientNameStr[userSessionInfo[sessionIdx].clientType], userSessionInfo[sessionIdx].ipAddr);
    }
    else if (sessionIdx == DI_CLIENT_SESSION)
    {
        snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, SAMAS_INTEGRATION_EVT_STR " | %s", userSessionInfo[sessionIdx].ipAddr);
    }

    userSessionInfo[sessionIdx].isSessionActive = FALSE;
    userSessionInfo[sessionIdx].sessionId = INVALID_CONNECTION;

    ReadSingleUserAccountConfig(userSessionInfo[sessionIdx].userIndex, &userAccountConfig);
    WriteEvent(LOG_USER_EVENT, LOG_USER_SESSION, userAccountConfig.username, advncdetail, EVENT_LOGOUT);

    userSessionInfo[sessionIdx].userIndex = MAX_USER_ACCOUNT;
    userSessionInfo[sessionIdx].viewerUserCfgIndex = MAX_USER_ACCOUNT;
    userSessionInfo[sessionIdx].viewerUserSysTk = 0;
    userSessionInfo[sessionIdx].viewerSessMultiLogin = FALSE;
    userSessionInfo[sessionIdx].reLoginOnMultiLoginDisable = FALSE;
    DPRINT(SYS_LOG, "user logout: [username=%s], [session=%d], [UserInfo=%s]", userAccountConfig.username, sessionIdx, advncdetail);

    MUTEX_UNLOCK(userSessionInfo[sessionIdx].userSessionLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Hold all activities of user
 * @param   sessionIdx
 */
void UserHold(UINT8 sessionIdx)
{
    if (FALSE == IsUserSessionActive(sessionIdx))
    {
        EPRINT(SYS_LOG, "user session is already inactive: [sessionIdx=%d]", sessionIdx);
        return;
    }

    /* Hold all playbacks; Live stream not paused but we can */
    PauseResumeSyncPlayBackFromSessionId(sessionIdx, TRUE);
    PauseResumePlaybackFromSessionId(sessionIdx, TRUE);
    PauseResumeInstantPlayBackFromSessionId(sessionIdx, TRUE);

    /* We should stop camera search also */
    StopCameraSearch(sessionIdx);
    StopAdvanceCameraSearch(sessionIdx);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Resume all activities of user
 * @param   sessionIdx
 */
void UserResume(UINT8 sessionIdx)
{
    if (FALSE == IsUserSessionActive(sessionIdx))
    {
        EPRINT(SYS_LOG, "user session is already inactive: [sessionIdx=%d]", sessionIdx);
        return;
    }

    /* Resume all playbacks; We have not paused live stream */
    PauseResumeSyncPlayBackFromSessionId(sessionIdx, FALSE);
    PauseResumePlaybackFromSessionId(sessionIdx, FALSE);
    PauseResumeInstantPlayBackFromSessionId(sessionIdx, FALSE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief       assign free session index
 * @param[out]  pSessionIdx
 * @return      if free session is available then CMD_SUCCESS else CMD_MAX_USER_SESSION
 */
static NET_CMD_STATUS_e allocateUserSession(UINT8PTR pSessionIdx)
{
    UINT8 session;

    for (session = (USER_LOCAL + 1); session < (MAX_NW_CLIENT -1); session++)
    {
        MUTEX_LOCK(userSessionInfo[session].userSessionLock);
        if(userSessionInfo[session].isSessionActive == TRUE)
        {
            MUTEX_UNLOCK(userSessionInfo[session].userSessionLock);
            continue;
        }
        userSessionInfo[session].isSessionActive = TRUE;
        MUTEX_UNLOCK(userSessionInfo[session].userSessionLock);

        /* occupy free session  */
        *pSessionIdx = session;
        return CMD_SUCCESS;
    }

    return CMD_MAX_USER_SESSION;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   deallocate session id
 * @param   sessionIdx
 */
static void deallocateUserSession(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_IF_INVLD_SESSION_ID(sessionIdx);
    MUTEX_LOCK(userSessionInfo[sessionIdx].userSessionLock);
    userSessionInfo[sessionIdx].isSessionActive = FALSE;
    MUTEX_UNLOCK(userSessionInfo[sessionIdx].userSessionLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check free user session available or not
 * @return  TRUE if free session available; FALSE otherwise
 */
BOOL IsFreeUserSessionAvailable(void)
{
    UINT8 session;

    for (session = (USER_LOCAL + 1); session < (MAX_NW_CLIENT -1); session++)
    {
        MUTEX_LOCK(userSessionInfo[session].userSessionLock);
        if(userSessionInfo[session].isSessionActive == FALSE)
        {
            MUTEX_UNLOCK(userSessionInfo[session].userSessionLock);
            return TRUE;
        }
        MUTEX_UNLOCK(userSessionInfo[session].userSessionLock);
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Give pointer to device initiation user session
 * @return
 */
USER_SESSION_INFO_t* GetDiUserSession(UINT8 sessionIdx)
{
    /* here sessionIdx will be DI_CLIENT_SESSION, last session number */
    return (&userSessionInfo[sessionIdx]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Decrement keep alive timer for user session.
 *          If timer expire log out user. If timer expire for local user reboot device.
 * @param   sessionIdx
 * @return  TRUE--> if success, FALSE--> if timer expire
 */
BOOL TickUserKeepAlive(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_FALSE_IF_INVLD_SESSION_ID(sessionIdx);

    /* Update keep-alive tick counter */
    userSessionInfo[sessionIdx].keepAliveTimerCnt--;
    if (userSessionInfo[sessionIdx].keepAliveTimerCnt)
    {
        /* Session is active */
        return TRUE;
    }

    /* No request received from local/remote device. Hence logout user */
    EPRINT(SYS_LOG, "keep-alive timeout: [sessionIdx=%d]", sessionIdx);
    UserLogout(sessionIdx);

    /* If it is local client then start preparation for power action restart */
    if (sessionIdx != USER_LOCAL)
    {
        /* Keep-alive timer is expired */
        return FALSE;
    }

    /* Get pid of GUI application and kill it if it is valid */
    INT32 pid = GetPidOfProcess(GUI_APPL_NAME);
    if (pid == NILL)
    {
        /* Ideally, it should not happen */
        EPRINT(SYS_LOG, "local client exited, so restart the system..!!");
        PrepForPowerAction(REBOOT_DEVICE, EVENT_AUTO, "UI");
    }
    else
    {
        /* On killing local client, nvr-x.sh script will start it again */
        EPRINT(SYS_LOG, "local client not responding, so restart it..!!");
        if (FAIL == KillProcessId(pid, KILL_SIG))
        {
            /* Ideally, it should not happen */
            EPRINT(SYS_LOG, "fail to kill local client, wait for some time...");
        }

        /* Start new timer for login wait. If login doesn't receive till timer expire then restart the system */
        TIMER_INFO_t startLoginWaitTimer;
        startLoginWaitTimer.count = CONVERT_SEC_TO_TIMER_COUNT(20);
        startLoginWaitTimer.data = (UINT32)pid;
        startLoginWaitTimer.funcPtr = localClientLoginWaitTmrCb;
        StartTimer(startLoginWaitTimer, &hLocalClientReloginMonitorTmr);
    }

    /* Keep-alive timer is expired */
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Decrement poll timer.
 * @param   sessionIdx
 * @return  TRUE--> if success, FALSE--> if timer expire
 */
BOOL TickUserPollTimer(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_FALSE_IF_INVLD_SESSION_ID(sessionIdx);

    userSessionInfo[sessionIdx].pollTimerCnt--;
    if (userSessionInfo[sessionIdx].pollTimerCnt == 0)
    {
        return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief reload keep alive timer as per value defined for client type
 * @param sessionIdx
 */
void ReloadUserKeepAlive(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_IF_INVLD_SESSION_ID(sessionIdx);
    userSessionInfo[sessionIdx].keepAliveTimerCnt = maxKeepAliveTime[userSessionInfo[sessionIdx].clientType];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief reload poll timer as per value defined for client type
 * @param sessionIdx
 */
void ReloadUserPollTimer(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_IF_INVLD_SESSION_ID(sessionIdx);
    userSessionInfo[sessionIdx].pollTimerCnt = MAX_POLL_TIMEOUT_DEVICE(maxPollTime[userSessionInfo[sessionIdx].clientType]);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Return status of session
 * @param   sessionIdx
 * @return
 */
BOOL IsUserSessionActive(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_FALSE_IF_INVLD_SESSION_ID(sessionIdx);
    return (userSessionInfo[sessionIdx].isSessionActive);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Validate user session
 * @param   sessionIdx
 * @param   sessionId
 * @return
 */
BOOL IsUserSessionValid(UINT8 sessionIdx, INT32 sessionId)
{
    /* Validate user session */
    RETURN_FALSE_IF_INVLD_SESSION_ID(sessionIdx);

    /* Validate allocated session id */
    if (userSessionInfo[sessionIdx].sessionId == INVALID_CONNECTION)
    {
        /* Session id not allocated */
        return FALSE;
    }

    /* Validate session id */
    if (userSessionInfo[sessionIdx].sessionId != sessionId)
    {
        /* session id not match. it is for other session */
        return FALSE;
    }

    /* Session is valid */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   check if polling is active for this user session
 * @param   sessionIdx
 * @return  TRUE--> if active, FALSE--> if inactive
 */
BOOL IsUserPollingActive(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_FALSE_IF_INVLD_SESSION_ID(sessionIdx);
    if (userSessionInfo[sessionIdx].pollFd == INVALID_CONNECTION)
    {
       return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Give current event read index
 * @param   sessionIdx
 * @return
 */
INT16 GetUserEventReadIndex(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_VAL_IF_INVLD_SESSION_ID(sessionIdx, 0);
    return userSessionInfo[sessionIdx].eventReadIndex;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Give client polling fd
 * @param   sessionIdx
 * @return
 */
INT32 GetUserPollFd(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_VAL_IF_INVLD_SESSION_ID(sessionIdx, INVALID_CONNECTION);
    return (userSessionInfo[sessionIdx].pollFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set polling fd
 * @param   sessionIdx
 * @param   pollfd
 * @return
 */
BOOL SetUserPollFd(UINT8 sessionIdx, INT32 pollfd)
{
    /* Validate user session */
    RETURN_FALSE_IF_INVLD_SESSION_ID(sessionIdx);
    userSessionInfo[sessionIdx].pollFd = pollfd;
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Give client type for given user session
 * @param sessionIdx
 * @return
 */
NW_CLIENT_TYPE_e GetUserClientType(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_VAL_IF_INVLD_SESSION_ID(sessionIdx, NW_CLIENT_TYPE_MAX);
    return (userSessionInfo[sessionIdx].clientType);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief give session id for given user session
 * @param sessionIdx
 * @return
 */
INT32 GetUserSessionId(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_VAL_IF_INVLD_SESSION_ID(sessionIdx, -1);
    return (userSessionInfo[sessionIdx].sessionId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief give user index for given user session
 * @param sessionIdx
 * @return
 */
UINT8 GetUserAccountIndex(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_VAL_IF_INVLD_SESSION_ID(sessionIdx, -1);
    return (userSessionInfo[sessionIdx].userIndex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief set user index for given user session
 * @param sessionIdx
 * @param userIdx
 */
void SetUserAccountIndex(UINT8 sessionIdx, UINT8 userIdx)
{
    /* Validate user session and user account index */
    RETURN_IF_INVLD_SESSION_ID(sessionIdx);
    RETURN_IF_INVLD_USER_ID(userIdx);
    userSessionInfo[sessionIdx].userIndex = userIdx;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief set ViewerUserCfgIndex
 * @param sessionIdx
 * @param cfgIndx
 */
void SetViewerUserCfgIndex(UINT8 sessionIdx, UINT8 cfgIndx)
{
    /* Validate user session */
    RETURN_IF_INVLD_SESSION_ID(sessionIdx);
    MUTEX_LOCK(userSessionInfo[sessionIdx].userSessionLock);
    userSessionInfo[sessionIdx].viewerUserCfgIndex = cfgIndx;
    MUTEX_UNLOCK(userSessionInfo[sessionIdx].userSessionLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief get ViewerUserSysTk
 * @param sessionIdx
 * @return
 */
UINT32 GetViewerUserSysTk(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_VAL_IF_INVLD_SESSION_ID(sessionIdx, 0);
    return (userSessionInfo[sessionIdx].viewerUserSysTk);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief set ViewerUserSysTk
 * @param sessionIdx
 * @param tick
 */
void SetViewerUserSysTk(UINT8 sessionIdx, UINT32 tick)
{
    /* Validate user session */
    RETURN_IF_INVLD_SESSION_ID(sessionIdx);
    MUTEX_LOCK(userSessionInfo[sessionIdx].userSessionLock);
    userSessionInfo[sessionIdx].viewerUserSysTk = tick;
    MUTEX_UNLOCK(userSessionInfo[sessionIdx].userSessionLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief get ViewerSessMultiLoginFlag
 * @param sessionIdx
 * @return
 */
BOOL GetViewerSessMultiLoginFlag(UINT8 sessionIdx)
{
    /* Validate user session */
    RETURN_FALSE_IF_INVLD_SESSION_ID(sessionIdx);
    return (userSessionInfo[sessionIdx].viewerSessMultiLogin);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief set ViewSessMultiLoginFlag
 * @param sessionIdx
 * @param flag
 */
void SetViewSessMultiLoginFlag(UINT8 sessionIdx, BOOL flag)
{
    /* Validate user session */
    RETURN_IF_INVLD_SESSION_ID(sessionIdx);
    MUTEX_LOCK(userSessionInfo[sessionIdx].userSessionLock);
    userSessionInfo[sessionIdx].viewerSessMultiLogin = flag;
    MUTEX_UNLOCK(userSessionInfo[sessionIdx].userSessionLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief get UserBlockStatus
 * @param userIdx
 * @return
 */
BOOL GetUserBlockStatus(UINT8 userIdx)
{
    /* Validate user account index */
    RETURN_FALSE_IF_INVLD_USER_ID(userIdx);
    return userBlocked[userIdx];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief set UserBlockStatus
 * @param userIdx
 * @param status
 */
void SetUserBlockStatus(UINT8 userIdx, BOOL status)
{
    /* Validate user account index */
    RETURN_IF_INVLD_USER_ID(userIdx);

    MUTEX_LOCK(userBlockMutex);
    userBlocked[userIdx] = status;
    MUTEX_UNLOCK(userBlockMutex);

    /* remove devcie to disable fcm push notification */
    if (TRUE == status)
    {
        /* get username from user index */
        USER_ACCOUNT_CONFIG_t userAccountConfig;
        ReadSingleUserAccountConfig(userIdx, &userAccountConfig);
        FcmPushDeleteSystemUserNotify(userAccountConfig.username);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief get ip address of logged in user
 * @param sessionIdx
 * @return
 */
CHARPTR GetUserIpAddr(UINT8 sessionIdx)
{
    return (userSessionInfo[sessionIdx].ipAddr);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief get UserMultiLoginFlag
 * @param sessionIdx
 * @return
 */
BOOL GetUserMultiLoginFlag(UINT8 sessionIdx)
{
    return (userSessionInfo[sessionIdx].reLoginOnMultiLoginDisable);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief set UserMultiLoginFlag
 * @param sessionIdx
 * @param status
 */
void SetUserMultiLoginFlag(UINT8 sessionIdx, BOOL status)
{
    /* Validate user session */
    RETURN_IF_INVLD_SESSION_ID(sessionIdx);
    MUTEX_LOCK(userSessionInfo[sessionIdx].userSessionLock);
    userSessionInfo[sessionIdx].reLoginOnMultiLoginDisable = status;
    MUTEX_UNLOCK(userSessionInfo[sessionIdx].userSessionLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is user config notify function
 * @param   newUsrAccontCfg
 * @param   oldUsrAccontCfg
 * @param   usrIndx
 */
void UserAccountCfgUpdate(USER_ACCOUNT_CONFIG_t newUsrAccontCfg, USER_ACCOUNT_CONFIG_t *oldUsrAccontCfg, UINT8 usrIndx)
{
    BOOL                        isUserListChanged = NO;
    USER_ACCOUNT_CONFIG_t       usrAccontCfg;
    USER_SESSION_FILE_INFO_t    viewerUserSessInfo;
    UINT8                       viewerUsrCfgIndex = 0;
    UINT8                       index = 0;
    BOOL                        passwordResetReq = FALSE;
    BOOL                        removeFcmNotificationDevice = FALSE;

    /* Check If user is deleted */
    if ((oldUsrAccontCfg->username[0] != '\0') && (newUsrAccontCfg.username[0] == '\0'))
    {
        /* Unblock the user */
        SetUserBlockStatus(usrIndx, FALSE);

        /* Delete client's layout file */
        DeleteClientDispLayoutFile(usrIndx);

        /* Default the user's password recovery config */
        DfltSinglePasswordRecoveryConfig(usrIndx);

        /* as user is deleted, remove associated device for fcm push notification */
        removeFcmNotificationDevice = TRUE;

        /* clearing password reset session of the user */
        clearUserPasswordResetSession(usrIndx);
    }

    if(oldUsrAccontCfg->userStatus != newUsrAccontCfg.userStatus)
    {
        if(newUsrAccontCfg.userStatus == DISABLE)
        {
            /* as user is disabled, remove associated device for fcm push notification */
            removeFcmNotificationDevice = TRUE;

            if(oldUsrAccontCfg->userGroup == ADMIN)
            {
                DPRINT(SYS_LOG, "delete user from system: [username=%s]", oldUsrAccontCfg->username);
                isUserListChanged = DeleteLinuxUser(oldUsrAccontCfg->username, usrIndx);
            }

            /* delete expiry password information */
            deletPasswordExpiryInfo(usrIndx);
            RemoveInvalidPassEntry(usrIndx);

            /* Default the user's password recovery config */
            DfltSinglePasswordRecoveryConfig(usrIndx);

			/* clearing password reset session of the user */
        	clearUserPasswordResetSession(usrIndx);
        }
        else
        {
            if(newUsrAccontCfg.userGroup == ADMIN)
            {
                DPRINT(SYS_LOG, "add user in system: [username=%s]", newUsrAccontCfg.username);
                isUserListChanged = AddLinuxUser(newUsrAccontCfg.username, newUsrAccontCfg.password);
            }
            else if(newUsrAccontCfg.userGroup == VIEWER)
            {
                if(IfUserAvailableInCfg(&viewerUsrCfgIndex, newUsrAccontCfg.username, &viewerUserSessInfo) == TRUE)
                {
                    DPRINT(SYS_LOG, "viewer user deleted and created new one");
                    viewerUserSessInfo.totalRemainingSessionTime = newUsrAccontCfg.loginSessionDuration;
                    viewerUserSessInfo.totalSessionTime = newUsrAccontCfg.loginSessionDuration;
                    viewerUserSessInfo.totalElapsedSessionTime = 0;
                    UpdateUserSessionData(viewerUsrCfgIndex, &viewerUserSessInfo);
                }
            }

            CreateNewPasswordExpiryInfo(usrIndx, &newUsrAccontCfg, TRUE);
            RemoveInvalidPassEntry(usrIndx);
        }
    }
    else if(newUsrAccontCfg.userStatus == ENABLE)
    {
        if(strcasecmp(oldUsrAccontCfg->username, newUsrAccontCfg.username) != 0)
        {
            if ((oldUsrAccontCfg->userGroup == ADMIN) && (oldUsrAccontCfg->username[0] != '\0'))
            {
                DPRINT(SYS_LOG, "delete user from system: [username=%s]", oldUsrAccontCfg->username);
                isUserListChanged = DeleteLinuxUser(oldUsrAccontCfg->username,usrIndx);
            }

            deletPasswordExpiryInfo(usrIndx);
            RemoveInvalidPassEntry(usrIndx);

            if(newUsrAccontCfg.username[0] != '\0')
            {
                if(newUsrAccontCfg.userGroup == ADMIN)
                {
                    DPRINT(SYS_LOG, "add user in system: [username=%s]", newUsrAccontCfg.username);
                    isUserListChanged = AddLinuxUser(newUsrAccontCfg.username, newUsrAccontCfg.password);
                }

                CreateNewPasswordExpiryInfo(usrIndx, &newUsrAccontCfg, TRUE);
                RemoveInvalidPassEntry(usrIndx);
            }
        }
        else if(strcasecmp(oldUsrAccontCfg->password, newUsrAccontCfg.password) != 0)
        {
            if ((oldUsrAccontCfg->userGroup == ADMIN) && (oldUsrAccontCfg->username[0] != '\0'))
            {
                DPRINT(SYS_LOG, "delete user from system: [username=%s]", oldUsrAccontCfg->username);
                isUserListChanged = DeleteLinuxUser(oldUsrAccontCfg->username,usrIndx);
            }

            deletPasswordExpiryInfo(usrIndx);
            RemoveInvalidPassEntry(usrIndx);

            if(newUsrAccontCfg.password[0] != '\0')
            {
                if(newUsrAccontCfg.userGroup == ADMIN)
                {
                    DPRINT(SYS_LOG, "add user in system: [username=%s]", newUsrAccontCfg.username);
                    isUserListChanged = AddLinuxUser(newUsrAccontCfg.username, newUsrAccontCfg.password);
                }

                CreateNewPasswordExpiryInfo(usrIndx, &newUsrAccontCfg, FALSE);
                RemoveInvalidPassEntry(usrIndx);
            }
        }
        else if(oldUsrAccontCfg->userGroup != newUsrAccontCfg.userGroup)
        {
            if(newUsrAccontCfg.userGroup == ADMIN)
            {
                DPRINT(SYS_LOG, "add user in system: [username=%s]", newUsrAccontCfg.username);
                isUserListChanged = AddLinuxUser(newUsrAccontCfg.username, newUsrAccontCfg.password);

                if(CheckForPasswordResetReq(usrIndx, &newUsrAccontCfg) == SUCCESS)
                {
                    passwordResetReq = TRUE;
                }
                CreateNewPasswordExpiryInfo(usrIndx, &newUsrAccontCfg, passwordResetReq);
                RemoveInvalidPassEntry(usrIndx);
            }
            else
            {
                if ((oldUsrAccontCfg->userGroup == ADMIN) && (oldUsrAccontCfg->username[0] != '\0'))
                {
                    DPRINT(SYS_LOG, "delete user from system: [username=%s]", oldUsrAccontCfg->username);
                    isUserListChanged = DeleteLinuxUser(oldUsrAccontCfg->username,usrIndx);
                }

                if(CheckForPasswordResetReq(usrIndx, &newUsrAccontCfg) == SUCCESS)
                {
                    passwordResetReq = TRUE;
                }

                deletPasswordExpiryInfo(usrIndx);
                RemoveInvalidPassEntry(usrIndx);

                CreateNewPasswordExpiryInfo(usrIndx, &newUsrAccontCfg, passwordResetReq);
                RemoveInvalidPassEntry(usrIndx);
            }
        }
        else if((newUsrAccontCfg.userGroup == VIEWER) &&
                        (oldUsrAccontCfg->loginSessionDuration != newUsrAccontCfg.loginSessionDuration))
        {
            if(IfUserAvailableInCfg(&viewerUsrCfgIndex, newUsrAccontCfg.username, &viewerUserSessInfo) == TRUE)
            {
                DPRINT(SYS_LOG, "account user available before changes: [totalTime=%d], [elapsedTime=%d]",
                        viewerUserSessInfo.totalSessionTime, viewerUserSessInfo.totalElapsedSessionTime);

                for(index = USER_LOCAL; index < (MAX_NW_CLIENT - 1); index++)
                {
                    /* Nothing to do if session is not active */
                    if (FALSE == IsUserSessionActive(index))
                    {
                        continue;
                    }

                    /* Deny User if another user is Logged in with same User Name */
                    ReadSingleUserAccountConfig(GetUserAccountIndex(index), &usrAccontCfg);
                    if (strcmp(usrAccontCfg.username, newUsrAccontCfg.username) == STATUS_OK)
                    {
                        break;
                    }
                }

                viewerUserSessInfo.totalSessionTime = newUsrAccontCfg.loginSessionDuration;
                if(viewerUserSessInfo.totalSessionTime <= viewerUserSessInfo.totalElapsedSessionTime)
                {
                    // log out
                    viewerUserSessInfo.totalRemainingSessionTime = 0;
                    if(index != (MAX_NW_CLIENT - 1))
                    {
                        CHAR eventAdvDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

                        /* Send live event of session expire */
                        snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", newUsrAccontCfg.username);
                        WriteEvent(LOG_USER_EVENT, LOG_SESSION_EXPIRE, NULL, eventAdvDetail, EVENT_ALERT);
                        sleep(1);
                        if(index != USER_LOCAL)
                        {
                            UserLogout(index);
                        }
                    }
                }

                DPRINT(SYS_LOG, "account user available after changes: [totalTime=%d], [elapsedTime=%d]",
                        viewerUserSessInfo.totalSessionTime, viewerUserSessInfo.totalElapsedSessionTime);
                UpdateUserSessionData(viewerUsrCfgIndex, &viewerUserSessInfo);
            }
        }
        else
        {
            EPRINT(SYS_LOG, "no changes related to admin account!");
        }
    }

    /* Check whether device is user is deleted/disabled or user's push rights are removed? then delete the device */
    if ((TRUE == removeFcmNotificationDevice) ||
            ((oldUsrAccontCfg->managePushNotificationRights == ENABLE) && (newUsrAccontCfg.managePushNotificationRights == DISABLE)))
    {
        /* delete device to disable fcm push notification */
        FcmPushDeleteSystemUserNotify(oldUsrAccontCfg->username);
    }

    /* Check FTP user list updation is required or not */
    if (isUserListChanged == NO)
    {
        return;
    }

#if !defined(RK3588_NVRH)
    /* Add all admin group users in FTP access list */
    CHAR adminUserList[(MAX_USER_ACCOUNT_USERNAME_WIDTH * MAX_USER_ACCOUNT) + strlen(UPDATE_USER_SCRIPT) + 1];
    UINT16 outLen = snprintf(adminUserList, sizeof(adminUserList), UPDATE_USER_SCRIPT);
    for(usrIndx = 0; usrIndx < MAX_USER_ACCOUNT; usrIndx++)
    {
        ReadSingleUserAccountConfig(usrIndx, &usrAccontCfg);
        if ((usrAccontCfg.userGroup != ADMIN) || (usrAccontCfg.username[0] == '\0'))
        {
            continue;
        }

        outLen += snprintf(adminUserList + outLen, sizeof(adminUserList) - outLen, " %s", usrAccontCfg.username);
        if(outLen > sizeof(adminUserList))
        {
            EPRINT(SYS_LOG, "more buffer size required for msg: [length=%d]", outLen);
            outLen = sizeof(adminUserList);
            break;
        }
    }

    if (FALSE == ExeSysCmd(TRUE, adminUserList))
    {
        EPRINT(SYS_LOG, "fail to exe update user script!");
    }
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Login policy config change notify
 * @param   newLoginPolicyCfg
 * @param   oldLoginPolicyCfg
 */
void LoginPolicyCfgUpdate(LOGIN_POLICY_CONFIG_t newLoginPolicyCfg, LOGIN_POLICY_CONFIG_t *oldLoginPolicyCfg)
{
    UINT8 userIdx;

    if (newLoginPolicyCfg.lockAccountStatus != oldLoginPolicyCfg->lockAccountStatus)
    {
        for (userIdx = 0; userIdx < MAX_USER_ACCOUNT; userIdx++)
        {
            RemoveInvalidPassEntry(userIdx);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function verify that auto lock timer is expiry or not.
 * @param   attemptType
 * @param   userId
 * @param   passLockDuration
 * @return  TRUE if auto lock timer started, else FALSE
 */
BOOL CheckAutoLockTmrForUser(INVALID_ATTEMPT_TYPE_e attemptType,  UINT8 userId, UINT32PTR passLockDuration)
{
    LOGIN_POLICY_CONFIG_t	loginPolicyCfg;
    time_t					localTime = 0;
    time_t					remainingTime = 0;

    ReadLoginPolicyConfig(&loginPolicyCfg);

    /* lock user password attemp info */
    MUTEX_LOCK(passwordAttemptInfo[attemptType][userId].passwordAttemptMutex);

    /* if auto lock timer is not started for user then return FALSE*/
    if (passwordAttemptInfo[attemptType][userId].resetTimer == 0)
    {
        /* unlock user password attemp info */
        MUTEX_UNLOCK(passwordAttemptInfo[attemptType][userId].passwordAttemptMutex);
        return FALSE;
    }

    GetLocalTimeInSec(&localTime);

    /* if current timer is exceeded reset timer */
    if (localTime > (time_t)passwordAttemptInfo[attemptType][userId].resetTimer)
    {
        DPRINT(SYS_LOG, "[localTime=%d], [resetTimer=%d], [autoLockTimer=%d]",
               (UINT32)localTime, passwordAttemptInfo[attemptType][userId].resetTimer, (SEC_IN_ONE_MIN * loginPolicyCfg.autoLockTimer));

        /* check how much time is remaining for auto lock */
        if ((localTime - passwordAttemptInfo[attemptType][userId].resetTimer) < (SEC_IN_ONE_MIN * (time_t)loginPolicyCfg.autoLockTimer))
        {
            remainingTime = (SEC_IN_ONE_MIN * loginPolicyCfg.autoLockTimer) - (localTime - passwordAttemptInfo[attemptType][userId].resetTimer);
            *passLockDuration = ((remainingTime / SEC_IN_ONE_MIN) + 1);
            DPRINT(SYS_LOG, "user auto lock is not expired: [remainingTime=%d]", (UINT32)((remainingTime / SEC_IN_ONE_MIN) + 1));

            /* unlock user password attemp info */
            MUTEX_UNLOCK(passwordAttemptInfo[attemptType][userId].passwordAttemptMutex);
            return TRUE;
        }
    }

    passwordAttemptInfo[attemptType][userId].resetTimer = 0;
    passwordAttemptInfo[attemptType][userId].invalidAttempts = 0;

    /* unlock user password attemp info */
    MUTEX_UNLOCK(passwordAttemptInfo[attemptType][userId].passwordAttemptMutex);
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function verify that user has created his account and then he/she has not set password any time.
 * @param   userId
 * @param   userAccountCfg
 * @return
 */
BOOL CheckForPasswordResetReq(UINT8 userId, USER_ACCOUNT_CONFIG_t *userAccountCfg)
{
    UINT32					fileOffset;
    INT32					fileFd;
    PASSWORD_RESET_INFO_t 	passwordResetInfo;

    fileFd = open(USER_PASSWORD_RESET_FILE, READ_ONLY_MODE, FILE_PERMISSION);
    if (fileFd == INVALID_FILE_FD)
    {
        EPRINT(SYS_LOG, "failed to open file: [file=%s], [err=%s]", USER_PASSWORD_RESET_FILE, STR_ERR);
        return FALSE;
    }

    fileOffset = (userId * sizeof(PASSWORD_RESET_INFO_t));
    if (lseek(fileFd, fileOffset, SEEK_SET) != fileOffset)
    {
        EPRINT(SYS_LOG, "failed to set file position: [file=%s], [position=%d], [err=%s]", USER_PASSWORD_RESET_FILE, fileOffset, STR_ERR);
        close(fileFd);
        return FALSE;
    }

    if (read(fileFd, &passwordResetInfo, sizeof(PASSWORD_RESET_INFO_t)) != sizeof(PASSWORD_RESET_INFO_t))
    {
        EPRINT(SYS_LOG, "failed to read file: [file=%s], [err=%s]", USER_PASSWORD_RESET_FILE, STR_ERR);
        close(fileFd);
        return FALSE;
    }

    close(fileFd);
    DPRINT(SYS_LOG, "[username=%s], [isPasswordResetReq=%d]", userAccountCfg->username, passwordResetInfo.isPasswordResetReq);
    return passwordResetInfo.isPasswordResetReq;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function validates input password and gives status.
 * @param   userId
 * @param   userAccountCfg
 * @param   minPassChar
 * @return
 */
NET_CMD_STATUS_e CheckPasswordPolicy(LOGIN_POLICY_CONFIG_t *loginPolicyCfg, USER_ACCOUNT_CONFIG_t *userAccountCfg, UINT8PTR minPassChar)
{
    UINT8           passwordStrength = 0;
    UINT8           cnt = 0, i = 0;
    const CHARPTR   passSpecialChar = ".,()[]:@!#$*/\\+";

    *minPassChar = 0;

    // minimum characters required
	if(strlen(userAccountCfg->password) < loginPolicyCfg->minPassLength)
    {
		*minPassChar = loginPolicyCfg->minPassLength;
		EPRINT(SYS_LOG, "password requires minimum characters for user-%s", userAccountCfg->username);
        return CMD_MIN_PASSWORD_CHAR_REQUIRED;
    }

    for(cnt = 0; cnt < strlen(userAccountCfg->password); cnt++)
    {
        if((userAccountCfg->password[cnt] >= 'A') && (userAccountCfg->password[cnt] <= 'Z'))
        {
            passwordStrength |= PASS_HAS_UPPERCASE;
        }
        else if((userAccountCfg->password[cnt] >= 'a') && (userAccountCfg->password[cnt] <= 'z'))
        {
            passwordStrength |= PASS_HAS_LOWERCASE;
        }
        else if((userAccountCfg->password[cnt] >= '0') && (userAccountCfg->password[cnt] <= '9'))
        {
            passwordStrength |= PASS_HAS_NUMBER;
        }
        else
        {
            for(i = 0; i < strlen(passSpecialChar); i++)
            {
                if(userAccountCfg->password[cnt] == passSpecialChar[i])
                {
                    passwordStrength |= PASS_HAS_SPECIAL_CHAR;
                    break;
                }
            }
        }
    }

    // strength checking
	if ((passwordStrength & HIGH_PASSWORD_STRENGTH_VAL) != HIGH_PASSWORD_STRENGTH_VAL)
	{
        EPRINT(SYS_LOG, "password strength not as per high policy: [user=%s], [passwordStrength=0x%x]", userAccountCfg->username, passwordStrength);
		return CMD_HIGH_PASSWORD_SEC_REQ;
	}
	return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function validates input password and gives status.
 * @param   userId
 * @param   userAccountCfg
 * @param   passwordResetReq
 */
void CreateNewPasswordExpiryInfo(UINT8 userId, USER_ACCOUNT_CONFIG_t *userAccountCfg, BOOL passwordResetReq)
{
    INT32						fileFd = INVALID_FILE_FD;
    UINT32						fileOffset;
    time_t                      currTime;
    PASSWORD_RESET_INFO_t 		passwordResetInfo;

    fileFd = open(USER_PASSWORD_RESET_FILE, READ_WRITE_MODE, FILE_PERMISSION);
    if (fileFd == INVALID_FILE_FD)
    {
        EPRINT(SYS_LOG, "failed to open file: [file=%s], [err=%s]", USER_PASSWORD_RESET_FILE, STR_ERR);
        return;
    }

    fileOffset = (userId * sizeof(PASSWORD_RESET_INFO_t));
    if (lseek(fileFd, fileOffset, SEEK_SET) == fileOffset)
    {
        snprintf(passwordResetInfo.username, MAX_USER_ACCOUNT_USERNAME_WIDTH, "%s", userAccountCfg->username);
        snprintf(passwordResetInfo.password, MAX_USER_ACCOUNT_PASSWORD_WIDTH, "%s", userAccountCfg->password);
        passwordResetInfo.isPasswordResetReq = passwordResetReq;
        GetLocalTimeInSec(&currTime);
        passwordResetInfo.timeSinceCreated = currTime;

        if(write(fileFd, &passwordResetInfo, sizeof(PASSWORD_RESET_INFO_t)) != sizeof(PASSWORD_RESET_INFO_t))
        {
            EPRINT(SYS_LOG, "failed to write file: [file=%s], [err=%s]", USER_PASSWORD_RESET_FILE, STR_ERR);
        }
    }
    else
    {
        EPRINT(SYS_LOG, "failed to set file position: [file=%s], [position=%d], [err=%s]", USER_PASSWORD_RESET_FILE, fileOffset, STR_ERR);
    }

    close(fileFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Reset password expiry information
 * @param   userId
 */
void RemoveInvalidPassEntry(UINT8 userId)
{
    INVALID_ATTEMPT_TYPE_e attemptType;

    for (attemptType = 0; attemptType < INVALID_ATTEMPT_TYPE_MAX; attemptType++)
    {
        MUTEX_LOCK(passwordAttemptInfo[attemptType][userId].passwordAttemptMutex);
        passwordAttemptInfo[attemptType][userId].invalidAttempts = 0;
        passwordAttemptInfo[attemptType][userId].resetTimer = 0;
        MUTEX_UNLOCK(passwordAttemptInfo[attemptType][userId].passwordAttemptMutex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function updates invalid attempt of login.
 * @param   attemptType
 * @param   userId
 * @param   passLockDuration
 * @param   remainingLoginAttempt
 * @return
 */
BOOL UpdateInvalidPassEntry(INVALID_ATTEMPT_TYPE_e attemptType, UINT8 userId, UINT32PTR passLockDuration, UINT8PTR remainingLoginAttempt)
{
    time_t                  currTime;
    LOGIN_POLICY_CONFIG_t	loginPolicyCfg;

    *passLockDuration = 0;
    *remainingLoginAttempt = 0;
    ReadLoginPolicyConfig(&loginPolicyCfg);

    if (loginPolicyCfg.lockAccountStatus == LOGIN_LOCK_DISABLE)
    {
        EPRINT(SYS_LOG, "lock account on invld creadential is not defined");
        return FALSE;
    }

    /* lock password attempt info */
    MUTEX_LOCK(passwordAttemptInfo[attemptType][userId].passwordAttemptMutex);

    passwordAttemptInfo[attemptType][userId].invalidAttempts++;
    if(passwordAttemptInfo[attemptType][userId].invalidAttempts < loginPolicyCfg.invalidLoginAttempt)
    {
        /* unlock password attempt info */
        MUTEX_UNLOCK(passwordAttemptInfo[attemptType][userId].passwordAttemptMutex);
        *remainingLoginAttempt = loginPolicyCfg.invalidLoginAttempt - passwordAttemptInfo[attemptType][userId].invalidAttempts;
        return FALSE;
    }

    GetLocalTimeInSec(&currTime);
    passwordAttemptInfo[attemptType][userId].resetTimer = currTime;
    *passLockDuration = loginPolicyCfg.autoLockTimer;

    /* unlock password attempt info */
    MUTEX_UNLOCK(passwordAttemptInfo[attemptType][userId].passwordAttemptMutex);

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function verify password reset time expire or not.
 * @param   userId
 * @param   userAccountCfg
 * @param   passResetInOneDay
 * @return
 */
BOOL IsPasswordResetTimeExpire(UINT8 userId, USER_ACCOUNT_CONFIG_t *userAccountCfg, UINT8PTR passResetInOneDay)
{
    INT32					fileFd;
    PASSWORD_RESET_INFO_t 	passwordResetInfo;
    time_t					currTime = { 0 };
    UINT32					fileOffset;
    UINT32					timeDiff;
    UINT32					orgResetTime;
    UINT32					expireInDays;
    LOGIN_POLICY_CONFIG_t	loginPolicyCfg;

    *passResetInOneDay = FALSE;
    ReadLoginPolicyConfig(&loginPolicyCfg);

    if (loginPolicyCfg.passResetStatus == DISABLE)
    {
        DPRINT(SYS_LOG, "password expiry time not defined in config");
        return FALSE;
    }

    fileFd = open(USER_PASSWORD_RESET_FILE, READ_ONLY_MODE, FILE_PERMISSION);
    if (fileFd == INVALID_FILE_FD)
    {
        EPRINT(SYS_LOG, "failed to open file: [file=%s], [err=%s]", USER_PASSWORD_RESET_FILE, STR_ERR);
        return FALSE;
    }

    fileOffset = (userId * sizeof(PASSWORD_RESET_INFO_t));
    if (lseek(fileFd, fileOffset, SEEK_SET) != fileOffset)
    {
        EPRINT(SYS_LOG, "failed to set file position: [file=%s], [position=%d], [err=%s]", USER_PASSWORD_RESET_FILE, fileOffset, STR_ERR);
        close(fileFd);
        return FALSE;
    }

    if (read(fileFd, &passwordResetInfo, sizeof(PASSWORD_RESET_INFO_t)) != sizeof(PASSWORD_RESET_INFO_t))
    {
        EPRINT(SYS_LOG, "failed to read file: [file=%s], [err=%s]", USER_PASSWORD_RESET_FILE, STR_ERR);
        close(fileFd);
        return FALSE;
    }

    close(fileFd);

    if (strcmp(passwordResetInfo.username, userAccountCfg->username) != STATUS_OK)
    {
        return FALSE;
    }

    GetLocalTimeInSec(&currTime);

    if ((UINT32)currTime > passwordResetInfo.timeSinceCreated)
    {
        timeDiff = ((UINT32)currTime - passwordResetInfo.timeSinceCreated);

        // in terms of days
        orgResetTime = SEC_IN_ONE_DAY * loginPolicyCfg.passResetPeriod;
        expireInDays = (orgResetTime - SEC_IN_ONE_DAY);

        DPRINT(SYS_LOG, "[currTime=%ld], [timeSinceCreated=%d], [timeDiff=%d], [orgResetTime=%d], [expireInDays=%d]",
                currTime, passwordResetInfo.timeSinceCreated, timeDiff, orgResetTime, expireInDays);

        if (timeDiff > orgResetTime)
        {
            return TRUE;
        }
        else if (timeDiff >= expireInDays)
        {
            *passResetInOneDay = TRUE;
        }
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function verify password reset time expire or not.
 */
void RemovePassExpiryFile(void)
{
    if (unlink(USER_PASSWORD_RESET_FILE) != STATUS_OK)
    {
        EPRINT(SYS_LOG, "File not Removed [%s]", USER_PASSWORD_RESET_FILE);
    }
}

/**
 * @brief   This Function verify password reset time expire or not.
 */
BOOL IsPassExpiryFilePresent(void)
{
	if (access(USER_PASSWORD_RESET_FILE, F_OK) == STATUS_OK)
	{
		return (TRUE);
	}
	return (FALSE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Give current event write index
 * @return  write index
 */
INT16 GetCurrentEventWriteIndex(void)
{
    MUTEX_LOCK(eventMutex);
    INT16 writeIdx = eventWriteIndex;
    MUTEX_UNLOCK(eventMutex);
    return writeIdx;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Give copy of event at current read index from given user session
 * @param   sessionIdx
 * @param   pEventLog
 */
void GetEventForCurrentReadIndex(UINT8 sessionIdx, EVENT_LOG_t *pEventLog)
{
    MUTEX_LOCK(eventMutex);
    memcpy(pEventLog, &eventBuffer[userSessionInfo[sessionIdx].eventReadIndex], sizeof(EVENT_LOG_t));
    userSessionInfo[sessionIdx].eventReadIndex++;
    if (userSessionInfo[sessionIdx].eventReadIndex >= MAX_EVENTS_IN_BUFFER)
    {
        /* Rollover the read index */
        userSessionInfo[sessionIdx].eventReadIndex = 0;
    }
    MUTEX_UNLOCK(eventMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback function for event notification.
 * @param   pEventLog
 */
void SendEvent(EVENT_LOG_t *pEventLog)
{
    UINT8 sessionIdx;

    MUTEX_LOCK(eventMutex);
    eventBuffer[eventWriteIndex++] = *pEventLog;
    if (eventWriteIndex >= MAX_EVENTS_IN_BUFFER)
    {
        eventWriteIndex = 0;
    }

    /* Device initiation excluded because it doesn't send the request of live events */
    for (sessionIdx = 0; sessionIdx < (MAX_NW_CLIENT - 1); sessionIdx++)
    {
        /* Nothing to do if session is not active */
        if (FALSE == userSessionInfo[sessionIdx].isSessionActive)
        {
            continue;
        }

        /* If read and write index are not same then there is space in buffer */
        if (eventWriteIndex != userSessionInfo[sessionIdx].eventReadIndex)
        {
            continue;
        }

        /* Client live event buffer overflowed. Few events will be missed by client */
        EPRINT(SYS_LOG, "live event buffer overflow: [sessionIdx=%d]", sessionIdx);

        /* Update read index to next otherwise buffer will become empty */
        userSessionInfo[sessionIdx].eventReadIndex++;
        if (userSessionInfo[sessionIdx].eventReadIndex >= MAX_EVENTS_IN_BUFFER)
        {
            /* Rollover the read index */
            userSessionInfo[sessionIdx].eventReadIndex = 0;
        }
    }
    MUTEX_UNLOCK(eventMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function will allocate session for password recovery to a user
 * @param   pCmdStr
 * @param   clientAddr
 * @param   clientSockFd
 * @param   clientCbType
 */
void AllocPasswordResetSession(CHARPTR *pPwdRstReq, SOCK_ADDR_INFO_u *clientAddr, INT32 clientSockFd, CLIENT_CB_TYPE_e clientCbType)
{
    INT32                       userIndex;
    BOOL                        isLocalClient = FALSE;
    CHAR                        replyMsg[MAX_REPLY_SZ];
    CHAR                        ipAddrStr[IPV6_ADDR_LEN_MAX] = "";
    CHAR                        username[MAX_USER_ACCOUNT_USERNAME_WIDTH] = "";
    CHAR                        smartCode[MAX_FIELD_VALUE_SIZE] = "";
    UINT8                       index;
    UINT16                      outLen = 0;
    UINT8                       sessionIdx = PWD_RST_SESSION_MAX;
    UINT32                      sessionId;
    UINT32                      passLockDuration;
    PASSWORD_RECOVERY_CONFIG_t  pwdRecoveryCfg;
    USER_ACCOUNT_CONFIG_t       userAccountCfg;
    NW_CLIENT_TYPE_e            clientType = NW_CLIENT_TYPE_MAX;
    NET_CMD_STATUS_e            cmdResp = CMD_SUCCESS;
    PWD_RST_MODE_MASK_e         pwdRstModeMask = PWD_RST_MODE_MASK_NONE;
    IP_NW_ADDR_u                ipAddrNw;

    do
    {
        /* Get IP address in network format from socket address */
        GetIpAddrNwFromSockAddr(clientAddr, &ipAddrNw);

        /* Get IP address in string format from socket address */
        GetIpAddrStrFromSockAddr(clientAddr, ipAddrStr, sizeof(ipAddrStr));

        /* Check if login request is from local client */
        if ((clientAddr->sockAddr.sa_family == AF_INET) && (ipAddrNw.ip4 == LOCAL_USER_IP))
        {
            /* It is local client */
            isLocalClient = TRUE;
        }
        else
        {
            /* Check if ip is in black listed or not */
            if ((cmdResp = validateIpAddress(clientAddr)) != CMD_SUCCESS)
            {
                DPRINT(NETWORK_MANAGER, "ip-addr blocked: [ip=%s]", ipAddrStr);
                break;
            }
        }

        /* assign free pwd reset session */
        cmdResp = allocatePwdRstSession(&sessionIdx);
        if (cmdResp != CMD_SUCCESS)
        {
            DPRINT(NETWORK_MANAGER, "no free pwd rst session available: [ip=%s]", ipAddrStr);
            break;
        }

        /* parse smart code for client type validation */
        if(ParseStr(pPwdRstReq, FSP, smartCode, MAX_FIELD_VALUE_SIZE) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        /* Get Client type from smart code */
        if (deriveClientType(smartCode, clientCbType, &clientType) != CMD_SUCCESS)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        /* parse username */
        if (ParseStr(pPwdRstReq, FSP, username, MAX_USER_ACCOUNT_USERNAME_WIDTH) == FAIL)
        {
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        /* Validate username */
        for (userIndex = 0; userIndex < MAX_USER_ACCOUNT; userIndex++)
        {
            ReadSingleUserAccountConfig(userIndex, &userAccountCfg);
            if  (strcmp(username, userAccountCfg.username) == STATUS_OK)
            {
                break;
            }
        }

        if (userIndex == MAX_USER_ACCOUNT)
        {
            DPRINT(NETWORK_MANAGER, "user not found: [username=%s]", username);
            cmdResp = CMD_INVALID_USERNAME;
            break;
        }

        /* Check if user's password reset session already undergoing */
        for (index = 0; index < PWD_RST_SESSION_MAX; index++)
        {
            MUTEX_LOCK(userPwdRstInfo[index].sessionLock);
            if ((userPwdRstInfo[index].isSessionActive == FALSE) || (userPwdRstInfo[index].userIndex != userIndex))
            {
                MUTEX_UNLOCK(userPwdRstInfo[index].sessionLock);
                continue;
            }
            MUTEX_UNLOCK(userPwdRstInfo[index].sessionLock);
            cmdResp = CMD_PWD_RST_ALREADY_IN_PROGRESS;
            break;
        }

        if (cmdResp == CMD_PWD_RST_ALREADY_IN_PROGRESS)
        {
            DPRINT(NETWORK_MANAGER, "usr pwd rst session already running: [userIndex=%d], [index=%d]", userIndex, index);
            break;
        }

        /* Check user is locked or not */
        if (CheckAutoLockTmrForUser(INVALID_ATTEMPT_TYPE_PWD_RESET, userIndex, &passLockDuration) == TRUE)
        {
            cmdResp = CMD_USER_ACCOUNT_LOCK;
            DPRINT(NETWORK_MANAGER, "user account locked: [userIndex=%d], [lockDuration=%umin]", userIndex, passLockDuration);
            break;
        }

        /* Check If User is Enabled or not */
        if(userAccountCfg.userStatus == DISABLE)
        {
            cmdResp = CMD_USER_DISABLED;
            DPRINT(NETWORK_MANAGER, "user account disabled: [userIndex=%d]", userIndex);
            break;
        }

        /* Check if user is blocked or not */
        if(GetUserBlockStatus(userIndex) == TRUE)
        {
            cmdResp = CMD_USER_BLOCKED;
            DPRINT(NETWORK_MANAGER, "user account blocked: [userIndex=%d]", userIndex);
            break;
        }

        /* Check if User's password recovery info configuration */
        ReadSinglePasswordRecoveryConfig(userIndex, &pwdRecoveryCfg);
        if (pwdRecoveryCfg.emailId[0] != '\0')
        {
            /* User can reset password through OTP */
            pwdRstModeMask |= PWD_RST_MODE_MASK_OTP;
        }

        if ((pwdRecoveryCfg.securityQuestionInfo[0].answer[0] != '\0') &&
                (pwdRecoveryCfg.securityQuestionInfo[1].answer[0] != '\0') && (pwdRecoveryCfg.securityQuestionInfo[2].answer[0] != '\0'))
        {
            /* User can reset password through question-answer */
            pwdRstModeMask |= PWD_RST_MODE_MASK_QA;
        }

        /* Check user has configured verification details or not */
        if (pwdRstModeMask == PWD_RST_MODE_MASK_NONE)
        {
            cmdResp = CMD_MISSING_VERIFICATION_DETAILS;
            DPRINT(NETWORK_MANAGER, "user's password recovery info not configured: [userIndex=%d]", userIndex);
            break;
        }

        /* Get Session-ID */
        sessionId = (TRUE == isLocalClient) ? GetUserSessionId(USER_LOCAL) : assignSessionId(sessionIdx);

        /* Storing pwd rst session info */
        MUTEX_LOCK(userPwdRstInfo[sessionIdx].sessionLock);
        userPwdRstInfo[sessionIdx].clientType = clientType;
        snprintf(userPwdRstInfo[sessionIdx].ipAddr, sizeof(userPwdRstInfo[sessionIdx].ipAddr), "%s", ipAddrStr);
        userPwdRstInfo[sessionIdx].sessionId = sessionId;
        userPwdRstInfo[sessionIdx].userIndex = userIndex;
        userPwdRstInfo[sessionIdx].isSessionActive = TRUE;
        userPwdRstInfo[sessionIdx].sessionTmrCnt = PWD_RST_TMR_CNT_MAX;
        MUTEX_UNLOCK(userPwdRstInfo[sessionIdx].sessionLock);

        /* Preparing Positive response */
        outLen = snprintf(replyMsg, MAX_REPLY_SZ, "%c%s%c%d%c%06d%c%d%c%s%c%d%c%d%c%d%c%c",
                          SOM, headerReq[RPL_PWD], FSP, cmdResp, FSP, sessionId, FSP, pwdRstModeMask, FSP,
                          pwdRecoveryCfg.emailId, FSP, pwdRecoveryCfg.securityQuestionInfo[0].questionId, FSP,
                          pwdRecoveryCfg.securityQuestionInfo[1].questionId, FSP, pwdRecoveryCfg.securityQuestionInfo[2].questionId, FSP, EOM);

        /* send login response to client */
        sendCmdCb[clientCbType](clientSockFd, (UINT8PTR)replyMsg, outLen, MESSAGE_REPLY_TIMEOUT);
        DPRINT(NETWORK_MANAGER, "password reset session allocated: [sessionIdx=%d], [user=%s], [ip=%s]", sessionIdx, username, ipAddrStr);
        return;

    } while(0);

    /* Free password reset session on failure if allocated */
    if (sessionIdx < PWD_RST_SESSION_MAX)
    {
        DeallocPasswordResetSession(sessionIdx);
    }

    /* Preparing Negative Response */
    outLen = snprintf(replyMsg, MAX_REPLY_SZ, "%c%s%c%d", SOM, headerReq[RPL_PWD], FSP, cmdResp);

    /* if user account is locked, sending lock duration along with status */
    if (CMD_USER_ACCOUNT_LOCK == cmdResp)
    {
        outLen += snprintf(replyMsg + outLen, MAX_REPLY_SZ - outLen, "%c%u", FSP, passLockDuration);
    }

    /* Adding footer */
    outLen += snprintf(replyMsg + outLen, MAX_REPLY_SZ - outLen, "%c%c", FSP, EOM);
    sendCmdCb[clientCbType](clientSockFd, (UINT8PTR)replyMsg, outLen, MESSAGE_REPLY_TIMEOUT);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief       checks whether a session ID is valid or not
 * @param       sessionId
 * @param[out]  pSessionIdx
 * @return
 */
BOOL IsPasswordResetSessionValid(INT32 sessionId, UINT8 *pSessionIdx)
{
    UINT8 index;
    for (index = 0; index < PWD_RST_SESSION_MAX; index++)
    {
        if ((userPwdRstInfo[index].isSessionActive == TRUE) && (userPwdRstInfo[index].sessionId == sessionId))
        {
            /* get the session Index of matched session */
            *pSessionIdx = index;
            return TRUE;
        }
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief checks timer for every active password reset session, decrement the timer count & dealloc session on timer expiry
 */
void CheckPasswordResetSession(void)
{
    UINT8 index;
    for (index = 0; index < PWD_RST_SESSION_MAX; index++)
    {
        if (userPwdRstInfo[index].isSessionActive == FALSE)
        {
            continue;
        }

        if (userPwdRstInfo[index].sessionTmrCnt)
        {
            /* decrement the timer counter */
            userPwdRstInfo[index].sessionTmrCnt--;
        }

        /* If timer expired */
        if (userPwdRstInfo[index].sessionTmrCnt == 0)
        {
            DeallocPasswordResetSession(index);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   making the passsword reset session on session index default
 * @param   sessionIdx
 */
void DeallocPasswordResetSession(UINT8 sessionIdx)
{
    DPRINT(NETWORK_MANAGER, "free password reset session: [sessionIdx=%d], [ip=%s]", sessionIdx, userPwdRstInfo[sessionIdx].ipAddr);
    MUTEX_LOCK(userPwdRstInfo[sessionIdx].sessionLock);
    userPwdRstInfo[sessionIdx].isSessionActive = FALSE;
    userPwdRstInfo[sessionIdx].userIndex = MAX_USER_ACCOUNT;
    userPwdRstInfo[sessionIdx].clientType = NW_CLIENT_TYPE_MAX;
    userPwdRstInfo[sessionIdx].sessionId = -1;
    userPwdRstInfo[sessionIdx].sessionTmrCnt = 0;
    userPwdRstInfo[sessionIdx].ipAddr[0] = '\0';
    userPwdRstInfo[sessionIdx].otp6Digit = 0;
    MUTEX_UNLOCK(userPwdRstInfo[sessionIdx].sessionLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   reloads password reset session timer count
 * @param   sessionIdx
 */
void ReloadPasswordResetExpTmr(UINT8 sessionIdx)
{
    MUTEX_LOCK(userPwdRstInfo[sessionIdx].sessionLock);
    if (userPwdRstInfo[sessionIdx].isSessionActive == TRUE)
    {
        userPwdRstInfo[sessionIdx].sessionTmrCnt = PWD_RST_TMR_CNT_MAX;
    }
    MUTEX_UNLOCK(userPwdRstInfo[sessionIdx].sessionLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Gets user index of user having provided password reset session index
 * @param   sessionIdx
 * @return  User Index
 */
UINT8 UserIndexFrmPwdRstSession(UINT8 sessionIdx)
{
    /* Validate session index */
    RETURN_VAL_IF_INVLD_PSWD_RST_SESSION_ID(sessionIdx, -1)
    return userPwdRstInfo[sessionIdx].userIndex;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get generate OTP from user's password reset session
 * @param   sessionIdx
 * @return  otp
 */
UINT32 GetPasswordResetOtp(UINT8 sessionIdx)
{
    return userPwdRstInfo[sessionIdx].otp6Digit;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Generate and store the OTP in user's password reset session
 * @param   sessionIdx
 * @return  otp
 */
UINT32 GeneratePasswordResetOtp(UINT8 sessionIdx)
{
    UINT32 otp6Digit = GetRandomNum() % 1000000;
    userPwdRstInfo[sessionIdx].otp6Digit = otp6Digit;
    return otp6Digit;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is an API to write events in event log for password reset functionality
 * @param   sessionIdx
 * @param   tableId
 * @param   eventState
 */
void WriteConfigChangeEvent(UINT8 sessionIdx, TBL_CFG_e tableId, LOG_EVENT_STATE_e eventState)
{
    CHAR                    eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR                    advncdetail[MAX_EVENT_ADVANCE_DETAIL_SIZE] = "";
    USER_ACCOUNT_CONFIG_t   userAccCfg;

    ReadSingleUserAccountConfig(userSessionInfo[sessionIdx].userIndex, &userAccCfg);
    GET_EVENT_CONFIG_DETAIL(eventDetail, MAX_EVENT_DETAIL_SIZE, tableId);

    if (strcmp(userSessionInfo[sessionIdx].ipAddr, LOCAL_CLIENT_IP_ADDR) == 0)
    {
        snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | %s", userAccCfg.username, LOCAL_CLIENT_EVT_STR);
    }
    else if (userSessionInfo[sessionIdx].clientType < NW_CLIENT_TYPE_MAX)
    {
        /* It is remote client and also add its IP in event log */
        snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | %s | %s",
                 userAccCfg.username, pClientNameStr[userSessionInfo[sessionIdx].clientType], userSessionInfo[sessionIdx].ipAddr);
    }

    WriteEvent(LOG_USER_EVENT, LOG_CONFIG_CHANGE, eventDetail, advncdetail, eventState);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is an API to write password reset config change event in event log
 * @param   sessionIdx
 * @param   eventState
 */
void WritePwdRstConfigChnageEvent(UINT8 sessionIdx, LOG_EVENT_STATE_e eventState)
{
    CHAR                    eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR                    advncdetail[MAX_EVENT_ADVANCE_DETAIL_SIZE] = "";
    USER_ACCOUNT_CONFIG_t   userAccCfg;

    ReadSingleUserAccountConfig(userPwdRstInfo[sessionIdx].userIndex, &userAccCfg);
    snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%s", userAccCfg.username);

    if (strcmp(userPwdRstInfo[sessionIdx].ipAddr, LOCAL_CLIENT_IP_ADDR) == 0)
    {
        snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, LOCAL_CLIENT_EVT_STR);
    }
    else if (userPwdRstInfo[sessionIdx].clientType < NW_CLIENT_TYPE_MAX)
    {
        snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | %s",
                 pClientNameStr[userPwdRstInfo[sessionIdx].clientType], userPwdRstInfo[sessionIdx].ipAddr);
    }

    WriteEvent(LOG_USER_EVENT, LOG_PASSWORD_RESET, eventDetail, advncdetail, eventState);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function initialize password expiry info and timer.
 * @return  SUCCESS/FAIL
 */
static void	initPasswordExpiryInfo(void)
{
    INT32					fileFd = INVALID_FILE_FD;
    UINT8					cnt = 0;
    PASSWORD_RESET_INFO_t 	passwordResetInfo;
    USER_ACCOUNT_CONFIG_t	userAccountCfg;
    const CHAR              *pPwdResetFileName = USER_PASSWORD_RESET_FILE;

    do
    {
        /* Is password expiry file present? */
        if (FALSE == IsPassExpiryFilePresent())
        {
            /* File is not preset */
            break;
        }

        /* Open file to validate data */
        fileFd = open(pPwdResetFileName, READ_ONLY_MODE, USR_RWE_GRP_RE_OTH_RE);
        if (fileFd == INVALID_FILE_FD)
        {
            EPRINT(SYS_LOG, "fail to open file: [file=%s], [err=%s]", pPwdResetFileName, STR_ERR);
            break;
        }

        /* Read default user data from file for validation */
        for(cnt = 0; cnt < MAX_DFLT_USER; cnt++)
        {
            if (read(fileFd, &passwordResetInfo, sizeof(PASSWORD_RESET_INFO_t)) != sizeof(PASSWORD_RESET_INFO_t))
            {
                EPRINT(SYS_LOG, "fail to read file: [user=%d], [file=%s], [err=%s]", cnt, pPwdResetFileName, STR_ERR);
                break;
            }
        }

        /* Close the file */
        close(fileFd);

        /* All user data is not present */
        if (cnt < MAX_DFLT_USER)
        {
            break;
        }

        DPRINT(SYS_LOG, "password reset file already present");
        return;

    }while(0);

    fileFd = open(pPwdResetFileName, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
    if (fileFd == INVALID_FILE_FD)
    {
        EPRINT(SYS_LOG, "failed to open file: [file=%s], [err=%s]", pPwdResetFileName, STR_ERR);
        return;
    }

    for(cnt = 0; cnt < MAX_USER_ACCOUNT; cnt++)
    {
        ReadSingleUserAccountConfig(cnt, &userAccountCfg);
        snprintf(passwordResetInfo.username, MAX_USER_ACCOUNT_USERNAME_WIDTH, "%s", userAccountCfg.username);
        snprintf(passwordResetInfo.password, MAX_USER_ACCOUNT_PASSWORD_WIDTH, "%s", userAccountCfg.password);
        passwordResetInfo.timeSinceCreated = 0;
        passwordResetInfo.isPasswordResetReq = TRUE;

        if(write(fileFd, &passwordResetInfo, sizeof(PASSWORD_RESET_INFO_t)) != sizeof(PASSWORD_RESET_INFO_t))
        {
            EPRINT(SYS_LOG, "failed to write file: [file=%s], [err=%s]", pPwdResetFileName, STR_ERR);
            break;
        }
    }

    /* Close the file */
    close(fileFd);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This thread listens for incoming fd transfer requests until module de-initialisation.
 * @param   threadArg
 * @return
 */
static VOIDPTR userTimerThread(VOIDPTR threadArg)
{
    UINT8 					usrIndex = 0;
    UINT8					tempViewerUserCfgIndex = 0;
    INT32 					elapsedTimeDiff;
    UINT32 					elapsedActualTimeInMin = 0;
    USER_SESSION_FILE_INFO_t userSessionData;
    LOG_EVENT_TYPE_e		evntType;
    LOG_EVENT_SUBTYPE_e		evSubType;
    LOG_EVENT_STATE_e		evntState;
    CHAR					eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR					eventAdvDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];
    struct tm 				systemTimeInTmStruct = { 0 };
    static struct tm 		storedTimeInTmStruct;

    THREAD_START("VIEWER_USER");
    memset(&userSessionData, 0, sizeof(userSessionData));
    GetLocalTimeInBrokenTm(&storedTimeInTmStruct);

    while(TRUE)
    {
        for (usrIndex = 0; usrIndex < (MAX_NW_CLIENT - 1); usrIndex++)
        {
            elapsedTimeDiff = 0;
            elapsedActualTimeInMin = 0;

            MUTEX_LOCK(userSessionInfo[usrIndex].userSessionLock);
            tempViewerUserCfgIndex = userSessionInfo[usrIndex].viewerUserCfgIndex;
            MUTEX_UNLOCK(userSessionInfo[usrIndex].userSessionLock);

            if (tempViewerUserCfgIndex == MAX_USER_ACCOUNT)
            {
                continue;
            }

            ReadUserSessionData(tempViewerUserCfgIndex, &userSessionData);
            if (userSessionInfo[usrIndex].viewerSessMultiLogin == FALSE)
            {
                MUTEX_LOCK(userSessionInfo[usrIndex].userSessionLock);
                elapsedTimeDiff = ElapsedTick(userSessionInfo[usrIndex].viewerUserSysTk);
                MUTEX_UNLOCK(userSessionInfo[usrIndex].userSessionLock);

                elapsedActualTimeInMin = ((elapsedTimeDiff * 10000) / (6000000));

                // if diff more than 1 min , store updated
                if((elapsedActualTimeInMin >= 1))
                {
                    MUTEX_LOCK(userSessionInfo[usrIndex].userSessionLock);
                    userSessionInfo[usrIndex].viewerUserSysTk = GetSysTick();
                    MUTEX_UNLOCK(userSessionInfo[usrIndex].userSessionLock);

                    userSessionData.totalElapsedSessionTime += elapsedActualTimeInMin;
                    userSessionData.totalRemainingSessionTime = (userSessionData.totalSessionTime - userSessionData.totalElapsedSessionTime);

                    DPRINT(SYS_LOG, "user login duration: [usrIndex=%d], [elapsed=%dmin]", usrIndex, userSessionData.totalElapsedSessionTime);

                    if((userSessionData.totalRemainingSessionTime <= 0) || (userSessionData.totalRemainingSessionTime > 1440))
                    {
                        userSessionData.totalRemainingSessionTime = 0;
                    }
                    UpdateUserSessionData(tempViewerUserCfgIndex, &userSessionData);

                    if(userSessionData.totalRemainingSessionTime == USER_SES_EXPIRE_FIRST_LOG_TIME)
                    {
                        evntType = LOG_USER_EVENT;
                        evSubType = LOG_ALLOWED_ACCESS;
                        evntState = EVENT_ALERT;
                        snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "1");
                        snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", userSessionData.username);
                        WriteEvent(evntType, evSubType, eventDetail, eventAdvDetail, evntState);
                    }

                    if(userSessionData.totalSessionTime <= userSessionData.totalElapsedSessionTime)
                    {
                        // send live event of session expire
                        evntType = LOG_USER_EVENT;
                        evSubType = LOG_SESSION_EXPIRE;
                        evntState = EVENT_ALERT;
                        snprintf(eventAdvDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s", userSessionData.username);
                        WriteEvent(evntType, evSubType, NULL, eventAdvDetail, evntState);
                        sleep(1);
                    }
                }
            }

            if(userSessionData.totalSessionTime <= userSessionData.totalElapsedSessionTime)
            {
                if(usrIndex != USER_LOCAL)
                {
                    UserLogout(usrIndex);
                }
            }
        }

        // check if day change
        GetLocalTimeInBrokenTm(&systemTimeInTmStruct);

        if((systemTimeInTmStruct.tm_mday != storedTimeInTmStruct.tm_mday) ||
                (systemTimeInTmStruct.tm_mon != storedTimeInTmStruct.tm_mon) ||
                (systemTimeInTmStruct.tm_year != storedTimeInTmStruct.tm_year))
        {
            // reset all user
            resetAllViewerUserOnDayChange();
            memcpy((void *)&storedTimeInTmStruct, (void *)&systemTimeInTmStruct, sizeof(struct tm));
        }
        sleep(5);
    }

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   resetAllViewerUserOnDayChange
 */
static void	resetAllViewerUserOnDayChange(void)
{
    UINT8 loop = 0;
    USER_ACCOUNT_CONFIG_t userAccountConfig;
    USER_SESSION_FILE_INFO_t userSessionData;

    ResetAllDefaultUser(FALSE);

    for(loop = 0; loop < (MAX_NW_CLIENT - 1); loop++)
    {
        MUTEX_LOCK(userSessionInfo[loop].userSessionLock);
        if(userSessionInfo[loop].isSessionActive == FALSE)
        {
            MUTEX_UNLOCK(userSessionInfo[loop].userSessionLock);
            continue;
        }

        ReadSingleUserAccountConfig(userSessionInfo[loop].userIndex, &userAccountConfig);
        if(userAccountConfig.userGroup != VIEWER)
        {
            MUTEX_UNLOCK(userSessionInfo[loop].userSessionLock);
            continue;
        }

        // fill total Acess time from user Account cfg
        snprintf(userSessionData.username, MAX_USER_ACCOUNT_USERNAME_WIDTH, "%s", userAccountConfig.username);
        userSessionData.totalRemainingSessionTime = userAccountConfig.loginSessionDuration;
        userSessionData.totalSessionTime = userAccountConfig.loginSessionDuration;
        userSessionData.totalElapsedSessionTime = 0;
        UpdateUserSessionData(userSessionInfo[loop].viewerUserCfgIndex, &userSessionData);
        userSessionInfo[loop].viewerUserSysTk = GetSysTick();
        MUTEX_UNLOCK(userSessionInfo[loop].userSessionLock);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function delets password expiry information from file for particular userId.
 * @param   userId
 * @return
 */
static BOOL	deletPasswordExpiryInfo(UINT8 userId)
{
    INT32					fileFd = INVALID_FILE_FD;
    UINT32					fileOffset;
    PASSWORD_RESET_INFO_t 	passwordResetInfo;

    fileFd = open(USER_PASSWORD_RESET_FILE, READ_WRITE_MODE, FILE_PERMISSION);
    if(fileFd == INVALID_FILE_FD)
    {
        EPRINT(SYS_LOG, "failed to open file: [file=%s], [err=%s]", USER_PASSWORD_RESET_FILE, STR_ERR);
        return FAIL;
    }

    fileOffset = (userId * sizeof(PASSWORD_RESET_INFO_t));
    if(lseek(fileFd, fileOffset, SEEK_SET) != fileOffset)
    {
        EPRINT(SYS_LOG, "failed to set file position: [file=%s], [position=%d], [err=%s]", USER_PASSWORD_RESET_FILE, fileOffset, STR_ERR);
        close(fileFd);
        return FAIL;
    }

    passwordResetInfo.username[0] = '\0';
    passwordResetInfo.password[0] = '\0';
    passwordResetInfo.timeSinceCreated = 0;
    passwordResetInfo.isPasswordResetReq = FALSE;

    if(write(fileFd, &passwordResetInfo, sizeof(PASSWORD_RESET_INFO_t)) != sizeof(PASSWORD_RESET_INFO_t))
    {
        EPRINT(SYS_LOG, "failed to write file: [file=%s], [err=%s]", USER_PASSWORD_RESET_FILE, STR_ERR);
        close(fileFd);
        return FAIL;
    }

    //Before return value close file
    close(fileFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function generate loginResponse for userSession
 * @param   replyMsg
 * @param   cmdResponse
 * @param   sessionId
 * @param   clientType
 * @param   passLockDuration
 * @param   passResetInOneDay
 * @param   userGroup
 * @param   replyMsgLen
 */
static void generateLoginResponse(CHARPTR replyMsg, NET_CMD_STATUS_e cmdResponse, UINT32 sessionId,
                                  NW_CLIENT_TYPE_e clientType, UINT32 passLockDuration, UINT32 passResetInOneDay,
                                  USER_GROUP_e userGroup, UINT8 remainingLoginAttempts, UINT32 replyMsgLen)
{
    NVR_DEVICE_INFO_t deviceInfo;

    /* Get device information for device initiation login */
    GetNvrDeviceInfo(&deviceInfo);

    /* Send login message to client */
    snprintf(replyMsg, replyMsgLen,
             "%c"		// SOM
             "%s"		// ACK_LOG
             "%c"		// FSP
             "%03d"		// login resp
             "%06d"		// session Id
             "%02d"		// software version
             "%02d"		// software revision
             "%02d"		// comm ver
             "%02d"		// comm rev
             "%02d"		// resp time
             "%02d"		// KLV time
             "%02d"		// max camera
             "%02d"		// max analog cam
             "%02d"		// max Ip camera
             "%02d"		// configured analog cam
             "%02d"		// configured Ip cam
             "%02d"		// max sensor input
             "%02d"		// max alarm output
             "%02d"		// max audio in
             "%01d"	 	// max audio out
             "%01d"		// no of HDD
             "%01d"		// no of NDD
             "%01d"		// no of LAN
             "%01d"		// VGA Port
             "%01d"		// HDMI1
             "%01d"		// HDMI2
             "%01d"		// CVBS Main
             "%01d"		// CVBS Spot
             "%01d"		// CVBS Spot Analog
             "%01d"		// PTZ for Analog
             "%01d"		// USB ports
             "%01d"		// Main Stream Analog Resolution
             "%01d"		// Sub Stream Analog Resolution
             "%01d"		// Video Standard
             "%04d"		// Max encoding Capacity
             "%04d"		// Max decoding Capacity
             "%01d"		// Disk checking status
             "%01d"		// Login user type
             "%03d"		// Password Policy Lock time
             "%01d"		// Password Expiration time
             "%02d"		// Product Variant
             "%02d"		// Product sub version
             "%02d"     // RemainingLoginAttempts
             "%s"		// Reserved 8 Bytes
             "%c"		// FSP
             "%c",		// EOM
             SOM, headerReq[ACK_LOG], FSP, cmdResponse,
             sessionId,
             deviceInfo.softwareVersion, deviceInfo.softwareRevision,
             deviceInfo.commVersion, deviceInfo.commRevision,
             maxPollTime[clientType], maxKeepAliveTime[clientType],
             deviceInfo.maxCameras, deviceInfo.maxAnalogCameras, deviceInfo.maxIpCameras,
             deviceInfo.configuredAnalogCameras, deviceInfo.configuredIpCameras,
             deviceInfo.maxSensorInput, deviceInfo.maxAlarmOutput,
             deviceInfo.audioIn, deviceInfo.audioOut,
             deviceInfo.noOfHdd, deviceInfo.noOfNdd, deviceInfo.noOfLanPort,
             deviceInfo.noOfVGA, deviceInfo.HDMI1, deviceInfo.HDMI2,
             deviceInfo.CVBSMain, deviceInfo.CVBSSpot,deviceInfo.CVBSSpotAnalog,
             deviceInfo.anlogPTZSupport, deviceInfo.USBPort, deviceInfo.maxMainAnalogResolution,
             deviceInfo.maxSubAnalogResolution, deviceInfo.videoStandard,
             deviceInfo.maxMainEncodingCap, deviceInfo.maxSubEncodingCap,
             deviceInfo.diskCheckingReq,
             userGroup,
             passLockDuration,
             passResetInOneDay,
             deviceInfo.productVariant,
             deviceInfo.productSubRevision,
             remainingLoginAttempts,
             pLoginReseverdBytes,
             FSP, EOM);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Validate user credentials, check for invalid password attmept and user lock
 * @param sessionIndex
 * @param userName
 * @param password
 * @param clientAddrStr
 * @param clientType
 * @param passLockDuration
 * @param passResetInOneDay
 * @param userGroup
 * @return
 */
static NET_CMD_STATUS_e validateUser(UINT8 sessionIndex,CHARPTR userName, CHARPTR password, CHARPTR clientAddrStr, NW_CLIENT_TYPE_e clientType,
                                     UINT32PTR passLockDuration, UINT8PTR passResetInOneDay, USER_GROUP_e *userGroup, UINT8PTR remainingLoginAttempts)
{
    CHAR                        *pPassStr;
    CHAR                        passHash[SHA256_STR_LEN_MAX];
    USER_ACCOUNT_CONFIG_t       userAccountConfig;
    NET_CMD_STATUS_e            cmdResponse = CMD_SUCCESS;
    UINT16                      index;
    UINT8                       viewerUsrCfgIndex = 0;
    USER_SESSION_FILE_INFO_t    viewerUserSessInfo;
    UINT32                      tempSysTick = 0;
    UINT8                       userIndex;

    do
    {
        if(sessionIndex == USER_LOCAL)
        {
            userIndex = USER_LOCAL;
            ReadSingleUserAccountConfig(userIndex, &userAccountConfig);
            break;
        }

        /* verify username */
        for(userIndex = (USER_LOCAL + 1); userIndex < MAX_USER_ACCOUNT; userIndex++)
        {
            ReadSingleUserAccountConfig(userIndex, &userAccountConfig);
            if(strcmp(userName, userAccountConfig.username) == STATUS_OK)
            {
                break;
            }
        }

        if(userIndex >= MAX_USER_ACCOUNT)
        {
            cmdResponse = CMD_INVALID_CREDENTIAL;
            break;
        }

        /* check if user is locked or not */
        if(CheckAutoLockTmrForUser(INVALID_ATTEMPT_TYPE_LOGIN, userIndex, passLockDuration) == TRUE)
        {
            EPRINT(SYS_LOG, "auto timer started so user block");
            cmdResponse = CMD_USER_ACCOUNT_LOCK;
            break;
        }

        /* In case of P2P, password will be in sha256 hash form */
        if ((clientType == NW_CLIENT_TYPE_P2P_MOBILE) || (clientType == NW_CLIENT_TYPE_P2P_DESKTOP))
        {
            /* Convert password in sha256 hash format and then compare */
            GetStrSha256(userAccountConfig.password, passHash);
            pPassStr = passHash;
        }
        else
        {
            pPassStr = userAccountConfig.password;
        }

        /* Verify password */
        if (strcmp(password, pPassStr) != STATUS_OK)
        {
            /* check if exceeded invalid password attempt */
            if (UpdateInvalidPassEntry(INVALID_ATTEMPT_TYPE_LOGIN, userIndex, passLockDuration, remainingLoginAttempts) == TRUE)
            {
                cmdResponse = CMD_USER_ACCOUNT_LOCK;
            }
            else
            {
                cmdResponse = CMD_INVALID_CREDENTIAL;
            }
            break;
        }

        // Check If User is Enabled or not
        if(userAccountConfig.userStatus == DISABLE)
        {
            cmdResponse = CMD_USER_DISABLED;
            break;
        }

        // Check if user is blocked or not
        if(GetUserBlockStatus(userIndex) == TRUE)
        {
            cmdResponse = CMD_USER_BLOCKED;
            break;
        }

        /* check if user is allowed for multi login */
        if(userAccountConfig.multiLogin == DISABLE)
        {
            for(index = USER_LOCAL; index < (MAX_NW_CLIENT - 1); index++)
            {
                /* deny if user is already logged in */
                MUTEX_LOCK(userSessionInfo[index].userSessionLock);
                if(userSessionInfo[index].userIndex != userIndex)
                {
                    MUTEX_UNLOCK(userSessionInfo[index].userSessionLock);
                    continue;
                }

                MUTEX_UNLOCK(userSessionInfo[index].userSessionLock);
                cmdResponse = CMD_MULTILOGIN;
                break;
            }
        }

        /* check for password reset request */
        if(CheckForPasswordResetReq(userIndex, &userAccountConfig) == TRUE)
        {
            cmdResponse = CMD_RESET_PASSWORD;
            MUTEX_LOCK(userSessionInfo[sessionIndex].userSessionLock);
            userSessionInfo[sessionIndex].isPassResetReq = TRUE;
            MUTEX_UNLOCK(userSessionInfo[sessionIndex].userSessionLock);
            WPRINT(SYS_LOG, "user has not changed its password after creating account");
            break;
        }

        // check for time has expired or not ...
        if(IsPasswordResetTimeExpire(userIndex, &userAccountConfig, passResetInOneDay) == TRUE)
        {
            cmdResponse = CMD_PASSWORD_EXPIRE;
            MUTEX_LOCK(userSessionInfo[sessionIndex].userSessionLock);
            userSessionInfo[sessionIndex].isPassResetReq = TRUE;
            MUTEX_UNLOCK(userSessionInfo[sessionIndex].userSessionLock);
            WPRINT(SYS_LOG, "password resetting required as per password policy");
            break;
        }

        // After successful login we should remove all invalid attempts.
        RemoveInvalidPassEntry(userIndex);

    }while(0);

    if((cmdResponse == CMD_SUCCESS) || (cmdResponse == CMD_RESET_PASSWORD) || (cmdResponse == CMD_PASSWORD_EXPIRE))
    {
        if((userAccountConfig.userGroup == VIEWER) && (strcmp(userAccountConfig.username, LOCAL_USER_NAME) != STATUS_OK))
        {
            viewerUsrCfgIndex =  MAX_USER_ACCOUNT;
            // check for user available in cfg
            if(IfUserAvailableInCfg(&viewerUsrCfgIndex, userAccountConfig.username, &viewerUserSessInfo) == TRUE)
            {
                DPRINT(SYS_LOG, "user available in cfg: [viewerUsrCfgIndex=%d], [totalSessionTime=%d], [totalElapsedSessionTime=%d]",
                       viewerUsrCfgIndex, viewerUserSessInfo.totalSessionTime,  viewerUserSessInfo.totalElapsedSessionTime);

                // if user found then check remaining time
                if(viewerUserSessInfo.totalSessionTime > viewerUserSessInfo.totalElapsedSessionTime)
                {
                    // fill nwSessionInfo with cfg index
                    MUTEX_LOCK(userSessionInfo[sessionIndex].userSessionLock);
                    userSessionInfo[sessionIndex].viewerUserCfgIndex = viewerUsrCfgIndex;
                    MUTEX_UNLOCK(userSessionInfo[sessionIndex].userSessionLock);

                    index = MAX_NW_CLIENT;
                    if(userAccountConfig.multiLogin == ENABLE)
                    {
                        for(index = USER_LOCAL; index < (MAX_NW_CLIENT - 1); index++)
                        {
                            // Deny User if another user is Logged in with same User Name
                            MUTEX_LOCK(userSessionInfo[index].userSessionLock);
                            if(userSessionInfo[index].userIndex != userIndex)
                            {
                                MUTEX_UNLOCK(userSessionInfo[index].userSessionLock);
                                continue;
                            }

                            tempSysTick = userSessionInfo[index].viewerUserSysTk;
                            MUTEX_UNLOCK(userSessionInfo[index].userSessionLock);
                            break;
                        }
                    }

                    if(index !=  (MAX_NW_CLIENT - 1))
                    {
                        // session already on, copy sys tick
                        DPRINT(SYS_LOG, "viewer same user session running: [index=%d]", index);
                        MUTEX_LOCK(userSessionInfo[sessionIndex].userSessionLock);
                        userSessionInfo[sessionIndex].viewerUserSysTk = tempSysTick;
                        userSessionInfo[sessionIndex].viewerSessMultiLogin = TRUE;
                        MUTEX_UNLOCK(userSessionInfo[sessionIndex].userSessionLock);
                    }
                    else
                    {
                        MUTEX_LOCK(userSessionInfo[sessionIndex].userSessionLock);
                        userSessionInfo[sessionIndex].viewerUserSysTk = GetSysTick();
                        MUTEX_UNLOCK(userSessionInfo[sessionIndex].userSessionLock);
                    }
                }
                else
                {
                    // give message session expire
                    cmdResponse = CMD_LOGIN_SESSION_DURATION_OVER;
                }
            }
            else
            {
                // If user not found then find free index in cfg
                if(FindFreeIndexInCfg(&viewerUsrCfgIndex) == TRUE)
                {
                    DPRINT(SYS_LOG, "viewer user not found in cfg, find free index: [viewerUsrCfgIndex=%d]", viewerUsrCfgIndex);
                    snprintf(viewerUserSessInfo.username, MAX_USER_ACCOUNT_USERNAME_WIDTH, "%s", userAccountConfig.username);
                    // fill total Acess time from user Account cfg
                    viewerUserSessInfo.totalRemainingSessionTime = userAccountConfig.loginSessionDuration;
                    viewerUserSessInfo.totalSessionTime = userAccountConfig.loginSessionDuration;
                    viewerUserSessInfo.totalElapsedSessionTime = 0;
                    UpdateUserSessionData(viewerUsrCfgIndex, &viewerUserSessInfo);

                    // fill nwSessionInfo with cfg index
                    MUTEX_LOCK(userSessionInfo[sessionIndex].userSessionLock);
                    userSessionInfo[sessionIndex].viewerUserCfgIndex = viewerUsrCfgIndex;
                    userSessionInfo[sessionIndex].viewerUserSysTk = GetSysTick();
                    MUTEX_UNLOCK(userSessionInfo[sessionIndex].userSessionLock);
                }
            }
        }
    }

    if( (cmdResponse == CMD_SUCCESS) || (cmdResponse == CMD_RESET_PASSWORD) || (cmdResponse == CMD_PASSWORD_EXPIRE) )
    {
        CHAR advncdetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

        MUTEX_LOCK(userSessionInfo[sessionIndex].userSessionLock);
        userSessionInfo[sessionIndex].isSessionActive = TRUE;
        userSessionInfo[sessionIndex].sessionId = assignSessionId(sessionIndex);
        userSessionInfo[sessionIndex].userIndex = userIndex;
        userSessionInfo[sessionIndex].clientType = clientType;
        snprintf(userSessionInfo[sessionIndex].ipAddr, sizeof(userSessionInfo[sessionIndex].ipAddr), "%s", clientAddrStr);
        MUTEX_UNLOCK(userSessionInfo[sessionIndex].userSessionLock);

        // Assign current write index to Read index of client
        MUTEX_LOCK(eventMutex);
        userSessionInfo[sessionIndex].eventReadIndex = eventWriteIndex;
        MUTEX_UNLOCK(eventMutex);

        // Load Keep alive timer count
        userSessionInfo[sessionIndex].keepAliveTimerCnt = maxKeepAliveTime[clientType];

        /* Derive client type. If IP is local host then it is local client. If IP is not local host and
         * client type is web client than it is device client else it is mobile client */
        advncdetail[0] = '\0';
        if (strcmp(clientAddrStr, LOCAL_CLIENT_IP_ADDR) == 0)
        {
            /* It is local client */
            snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, LOCAL_CLIENT_EVT_STR);
        }
        else if (clientType < NW_CLIENT_TYPE_MAX)
        {
            /* It is remote client and also add its IP in event log */
            snprintf(advncdetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "%s | %s", pClientNameStr[clientType], clientAddrStr);
        }

        WriteEvent(LOG_USER_EVENT, LOG_USER_SESSION, userAccountConfig.username, advncdetail, EVENT_LOGIN);
        DPRINT(SYS_LOG, "user logged in: [username=%s], [ip=%s], [sessionIdx=%d], [sessionKey=%d]",
               userAccountConfig.username, clientAddrStr, sessionIndex, userSessionInfo[sessionIndex].sessionId);
    }
    else
    {
        MUTEX_LOCK(userSessionInfo[sessionIndex].userSessionLock);
        userSessionInfo[sessionIndex].sessionId = 0;
        MUTEX_UNLOCK(userSessionInfo[sessionIndex].userSessionLock);

        WriteEvent(LOG_USER_EVENT, LOG_LOGIN_REQUEST, userName, clientAddrStr, EVENT_FAIL);
        EPRINT(SYS_LOG, "login failed: [ip=%s], [status=%d]", clientAddrStr, cmdResponse);

        /* login failed, so free the occupied session */
        MUTEX_LOCK(userSessionInfo[sessionIndex].userSessionLock);
        userSessionInfo[sessionIndex].isSessionActive = FALSE;
        MUTEX_UNLOCK(userSessionInfo[sessionIndex].userSessionLock);
    }

    *userGroup = userAccountConfig.userGroup;
    return cmdResponse;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function generates a unique session id.
 * @param   sessionIdx
 * @return  Unique session ID
 */
static INT32 assignSessionId(UINT8 sessionIdx)
{
    time_t	system_time;
    INT32	sessionId;

    /* Find time */
    if (time(&system_time) != NILL)
    {
        /* Cast upto month because we want 6 digit max */
        sessionId = system_time % 3600;

        /* Now, mask with our session index because we want unique in one second */
        sessionId = ((sessionId << 8) | sessionIdx);
        if ((sessionId != INVALID_CONNECTION) || (sessionId != 0))
        {
            return sessionId;
        }
    }

    do
    {
        /* Get random number and truncate to 6 digit max */
        sessionId = GetRandomNum() % 3600;

        /* Now, mask with our session index because we want unique id */
        sessionId = ((sessionId << 8) | sessionIdx);

    /* Generate number should not -1 or 0 */
    }while((sessionId == INVALID_CONNECTION) || (sessionId == 0));

    /* Unique random number */
    return sessionId;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function calls for validation of IP Address.
 * @param   ipAddrNw
 * @return  Network status
 */
static NET_CMD_STATUS_e validateIpAddress(SOCK_ADDR_INFO_u *ipAddrInfo)
{
    UINT32 				startIpAddress[IPV6_NW_ADDR_U32_SIZE];
    UINT32 				endIpAddress[IPV6_NW_ADDR_U32_SIZE];
    IP_FILTER_CONFIG_t 	ipFilterConfig;
    UINT16 				index;
    UINT32              ipAddress[IPV6_NW_ADDR_U32_SIZE] = { 0 };
    UINT8               tIndex;

    if (ipAddrInfo->sockAddr.sa_family == AF_INET)
    {
        memcpy(ipAddress, &ipAddrInfo->sockAddr4.sin_addr.s_addr, sizeof(struct in_addr));
    }
    else
    {
        memcpy(ipAddress, &ipAddrInfo->sockAddr6.sin6_addr, sizeof(struct in6_addr));
    }

    ReadIpFilterConfig(&ipFilterConfig);
    if (ipFilterConfig.ipFilter == FALSE)
    {
        return CMD_SUCCESS;
    }

    for(index = 0; index < MAX_IP_FILTER; index++)
    {
        memset(startIpAddress, 0, sizeof(startIpAddress));
        memset(endIpAddress, 0, sizeof(endIpAddress));

        /* If entries are blank then ignore it */
        if ((ipFilterConfig.filter[index].startAddress[0] == '\0') || (ipFilterConfig.filter[index].endAddress[0] == '\0'))
        {
            continue;
        }

        if (NM_IPADDR_FAMILY_V4 == NMIpUtil_GetIpAddrFamily(ipFilterConfig.filter[index].startAddress))
        {
            /* If mismatch between IP address & filter family type then continue */
            if (ipAddrInfo->sockAddr.sa_family != AF_INET)
            {
                continue;
            }
        }
        else
        {
            /* If mismatch between IP address & filter family type then continue */
            if (ipAddrInfo->sockAddr.sa_family != AF_INET6)
            {
                continue;
            }
        }

        inet_pton(ipAddrInfo->sockAddr.sa_family, ipFilterConfig.filter[index].startAddress, &startIpAddress);
        inet_pton(ipAddrInfo->sockAddr.sa_family, ipFilterConfig.filter[index].endAddress, &endIpAddress);

        /* Validate if IP falls within gievn range */
        for (tIndex = 0; tIndex < IPV6_NW_ADDR_U32_SIZE; tIndex++)
        {
            /* If IP out of range then check for next filter */
            if ((ntohl(ipAddress[tIndex]) < ntohl(startIpAddress[tIndex])) || (ntohl(ipAddress[tIndex]) > ntohl(endIpAddress[tIndex])))
            {
                break;
            }

            /* Return status if IP falls within range */
            if ((ntohl(ipAddress[tIndex]) > ntohl(startIpAddress[tIndex])) && (ntohl(ipAddress[tIndex]) < ntohl(endIpAddress[tIndex])))
            {
                return ((ipFilterConfig.mode == FILTERED_ALLOW) ? CMD_SUCCESS : CMD_IP_BLOCKED);
            }
        }

        if (tIndex == IPV6_NW_ADDR_U32_SIZE)
        {
            return ((ipFilterConfig.mode == FILTERED_ALLOW) ? CMD_SUCCESS : CMD_IP_BLOCKED);
        }
    }

    return ((ipFilterConfig.mode == FILTERED_ALLOW) ? CMD_IP_BLOCKED : CMD_SUCCESS);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Local client login wait timer callback
 * @param   data
 */
static void localClientLoginWaitTmrCb(UINT32 data)
{
    /* Delete the login wait timer */
    DeleteTimer(&hLocalClientReloginMonitorTmr);

    /* Restart rockchip device only if UI pid is not found or pid is same as previous */
    #if defined(RK3568_NVRL) || defined(RK3588_NVRH)
    INT32 pid = GetPidOfProcess(GUI_APPL_NAME);
    if ((pid != NILL) && (pid != (INT32)data))
    {
        /* We have restarted local client but display is not attached */
        WPRINT(SYS_LOG, "local client is running and waiting for display...");
    }
    else
    #endif
    {
        EPRINT(SYS_LOG, "local client login wait timer is expired, so restart the system...");
        PrepForPowerAction(REBOOT_DEVICE, EVENT_AUTO, "UI");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief       assign free password reset session index
 * @param[out]  pSessionIdx
 * @return      if free session is available or Not
 */
static NET_CMD_STATUS_e allocatePwdRstSession(UINT8PTR pSessionIdx)
{
    UINT8 sessionIdx;

    for (sessionIdx = 0; sessionIdx < PWD_RST_SESSION_MAX; sessionIdx++)
    {
        MUTEX_LOCK(userPwdRstInfo[sessionIdx].sessionLock);
        if(userPwdRstInfo[sessionIdx].isSessionActive == TRUE)
        {
            MUTEX_UNLOCK(userPwdRstInfo[sessionIdx].sessionLock);
            continue;
        }
        userPwdRstInfo[sessionIdx].isSessionActive = TRUE;
        MUTEX_UNLOCK(userPwdRstInfo[sessionIdx].sessionLock);

        /* occupy free session  */
        *pSessionIdx = sessionIdx;
        return CMD_SUCCESS;
    }

    return CMD_PWD_RST_SESSION_NOT_AVAILABLE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief This function parses recieved msg and derive smart code & Validates it as well
 * @param       smartCode
 * @param       clientCbType
 * @param[out]  clientType
 * @return
 */
static NET_CMD_STATUS_e deriveClientType(CHARPTR smartCode, CLIENT_CB_TYPE_e clientCbType, NW_CLIENT_TYPE_e *pClientType)
{
    /* validate client smart code  */
    if (strcmp(smartCode, SMART_CODE) == STATUS_OK)
    {
        if (clientCbType == CLIENT_CB_TYPE_NATIVE)
        {
            *pClientType = NW_CLIENT_TYPE_WEB;
        }
    }
    else if(strcmp(smartCode, MV_SMART_CODE) == STATUS_OK)
    {
        if (clientCbType == CLIENT_CB_TYPE_NATIVE)
        {
            *pClientType = NW_CLIENT_TYPE_MOBILE;
        }
    }
    else
    {
        return CMD_INVALID_CREDENTIAL;
    }

    /* Validate client type. It should be proper */
    if (*pClientType >= NW_CLIENT_TYPE_MAX)
    {
        EPRINT(SYS_LOG, "invld client type: [clientCbType=%d]", clientCbType);
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief if user's password session is active, deallocates the session
 * @param usrIndx
 */
static void clearUserPasswordResetSession(UINT8 usrIndx)
{
    UINT8 sessionIdx;
    for (sessionIdx = 0; sessionIdx < PWD_RST_SESSION_MAX; sessionIdx++)
    {
        if ((userPwdRstInfo[sessionIdx].isSessionActive == TRUE) && (userPwdRstInfo[sessionIdx].userIndex == usrIndx))
        {
            DeallocPasswordResetSession(sessionIdx);
            return;
        }
    }
}
//#################################################################################################
// @END OF FILE
//#################################################################################################
