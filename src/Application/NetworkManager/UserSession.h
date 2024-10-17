#ifndef USER_SESSION_H
#define USER_SESSION_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		UserSession.h
@brief      File maintains user sessions for all clients. Any Network module can request for user
            login/logout session.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Config.h"
#include "EventLogger.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define	INVALID_SESSION_INDEX       255
#define	MAX_NW_CLIENT				11

/* Last session number for samas integration */
#define DI_CLIENT_SESSION           (MAX_NW_CLIENT - 1)

#define	SOM							0x01
#define EOM							0x04
#define SOT							0x02
#define EOT							0x03
#define SOI							0x1C
#define EOI							0x1D
#define FSP							0x1E
#define FVS							0x1F

#define SMART_CODE 					"28573"
#define MV_SMART_CODE 				"28575"	// Smart code for mobile-viewer
#define SMART_CODE_COSEC			"CSI16"

#define LOCAL_CLIENT_IP_ADDR        "127.0.0.1"
#define PWD_RST_SESSION_MAX         (10)
#define PWD_RST_TMR_CNT_MAX         (180)   // secs

//#################################################################################################
// @DATA TYEPS
//#################################################################################################
typedef enum
{
    NW_CLIENT_TYPE_MOBILE = 0,
    NW_CLIENT_TYPE_WEB,
    NW_CLIENT_TYPE_P2P_MOBILE,
    NW_CLIENT_TYPE_P2P_DESKTOP,
    NW_CLIENT_TYPE_MAX
}NW_CLIENT_TYPE_e;

typedef enum
{
    INVALID_ATTEMPT_TYPE_LOGIN = 0,
    INVALID_ATTEMPT_TYPE_PWD_RESET,
    INVALID_ATTEMPT_TYPE_MAX,
}INVALID_ATTEMPT_TYPE_e;

typedef struct
{
    BOOL 				isSessionActive;
    BOOL 				isPassResetReq;
    UINT8 				userIndex;
    NW_CLIENT_TYPE_e	clientType;
    INT32 				sessionId;
    INT32 				pollFd;
    INT16				eventReadIndex;
    UINT16				pollTimerCnt;
    UINT16				keepAliveTimerCnt;
    CHAR 				ipAddr[IPV6_ADDR_LEN_MAX];
    UINT8				viewerUserCfgIndex;
    pthread_mutex_t		userSessionLock;
    UINT32				viewerUserSysTk;
    BOOL				viewerSessMultiLogin;
    BOOL			    reLoginOnMultiLoginDisable;

}USER_SESSION_INFO_t;

typedef struct
{
    BOOL 				isSessionActive;
    UINT8 				userIndex;
    CHAR 				ipAddr[IPV6_ADDR_LEN_MAX];
    UINT16				sessionTmrCnt;	
    INT32 				sessionId;
    UINT32              otp6Digit;
	NW_CLIENT_TYPE_e	clientType;    
    pthread_mutex_t		sessionLock;  

}USER_PWD_RST_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitUserSession(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ProcessClientLoginRequest(CHARPTR loginReq, SOCK_ADDR_INFO_u *clientAddr, UINT32 connFd, CLIENT_CB_TYPE_e clientCbType,
                                           NW_CLIENT_TYPE_e clientType, UINT8PTR sessionIdx);
//-------------------------------------------------------------------------------------------------
void UserLogout(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
void UserHold(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
void UserResume(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
BOOL IsFreeUserSessionAvailable(void);
//-------------------------------------------------------------------------------------------------
USER_SESSION_INFO_t* GetDiUserSession(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
BOOL TickUserKeepAlive(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
BOOL TickUserPollTimer(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
void ReloadUserKeepAlive(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
void ReloadUserPollTimer(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
BOOL IsUserSessionActive(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
BOOL IsUserSessionValid(UINT8 sessionIdx, INT32 sessionId);
//-------------------------------------------------------------------------------------------------
BOOL IsUserPollingActive(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
INT16 GetUserEventReadIndex(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
INT32 GetUserPollFd(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
BOOL SetUserPollFd(UINT8 sessionIdx, INT32 pollfd);
//-------------------------------------------------------------------------------------------------
NW_CLIENT_TYPE_e GetUserClientType(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
INT32 GetUserSessionId(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
UINT8 GetUserAccountIndex(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
void SetUserAccountIndex(UINT8 sessionIdx, UINT8 userIdx);
//-------------------------------------------------------------------------------------------------
void SetViewerUserCfgIndex(UINT8 sessionIdx, UINT8 cfgIndx);
//-------------------------------------------------------------------------------------------------
UINT32 GetViewerUserSysTk(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
void SetViewerUserSysTk(UINT8 sessionIdx, UINT32 tick);
//-------------------------------------------------------------------------------------------------
BOOL GetViewerSessMultiLoginFlag(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
void SetViewSessMultiLoginFlag(UINT8 sessionIdx, BOOL flag);
//-------------------------------------------------------------------------------------------------
BOOL GetUserBlockStatus(UINT8 userIdx);
//-------------------------------------------------------------------------------------------------
void SetUserBlockStatus(UINT8 userIdx, BOOL status);
//-------------------------------------------------------------------------------------------------
CHARPTR GetUserIpAddr(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
BOOL GetUserMultiLoginFlag(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
void SetUserMultiLoginFlag(UINT8 sessionIdx, BOOL status);
//-------------------------------------------------------------------------------------------------
void UserAccountCfgUpdate(USER_ACCOUNT_CONFIG_t newUsrAccontCfg, USER_ACCOUNT_CONFIG_t *oldUsrAccontCfg, UINT8 usrIndx);
//-------------------------------------------------------------------------------------------------
void LoginPolicyCfgUpdate(LOGIN_POLICY_CONFIG_t newLoginPolicyCfg, LOGIN_POLICY_CONFIG_t *oldLoginPolicyCfg);
//-------------------------------------------------------------------------------------------------
BOOL CheckAutoLockTmrForUser(INVALID_ATTEMPT_TYPE_e attemptType, UINT8 userId, UINT32PTR passLockDuration);
//-------------------------------------------------------------------------------------------------
BOOL CheckForPasswordResetReq(UINT8 userId, USER_ACCOUNT_CONFIG_t *userAccountCfg);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e CheckPasswordPolicy(LOGIN_POLICY_CONFIG_t *loginPolicyCfg, USER_ACCOUNT_CONFIG_t *userAccountCfg, UINT8PTR minPassChar);
//-------------------------------------------------------------------------------------------------
void CreateNewPasswordExpiryInfo(UINT8 userId, USER_ACCOUNT_CONFIG_t *userAccountCfg, BOOL passwordResetReq);
//-------------------------------------------------------------------------------------------------
void RemoveInvalidPassEntry(UINT8 userId);
//-------------------------------------------------------------------------------------------------
BOOL UpdateInvalidPassEntry(INVALID_ATTEMPT_TYPE_e attemptType, UINT8 userId, UINT32PTR passLockDuration, UINT8PTR remainingLoginAttempt);
//-------------------------------------------------------------------------------------------------
BOOL IsPasswordResetTimeExpire(UINT8 userId, USER_ACCOUNT_CONFIG_t *userAccountCfg, UINT8PTR passResetInOneDay);
//-------------------------------------------------------------------------------------------------
void RemovePassExpiryFile(void);
//-------------------------------------------------------------------------------------------------
BOOL IsPassExpiryFilePresent(void);
//-------------------------------------------------------------------------------------------------
INT16 GetCurrentEventWriteIndex(void);
//-------------------------------------------------------------------------------------------------
void GetEventForCurrentReadIndex(UINT8 sessionIdx, EVENT_LOG_t *pEventLog);
//-------------------------------------------------------------------------------------------------
void SendEvent(EVENT_LOG_t *pEventLog);
//-------------------------------------------------------------------------------------------------
void AllocPasswordResetSession(CHARPTR *pCmdStr, SOCK_ADDR_INFO_u *clientAddr, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
BOOL IsPasswordResetSessionValid(INT32 sessionId, UINT8 *pSessionIdx);
//-------------------------------------------------------------------------------------------------
void CheckPasswordResetSession(void);
//-------------------------------------------------------------------------------------------------
void DeallocPasswordResetSession(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
void ReloadPasswordResetExpTmr(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
UINT8 UserIndexFrmPwdRstSession(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
UINT32 GetPasswordResetOtp(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
UINT32 GeneratePasswordResetOtp(UINT8 sessionIdx);
//-------------------------------------------------------------------------------------------------
void WriteConfigChangeEvent(UINT8 sessionIdx, TBL_CFG_e tableId, LOG_EVENT_STATE_e eventState);
//-------------------------------------------------------------------------------------------------
void WritePwdRstConfigChnageEvent(UINT8 sessionIdx, LOG_EVENT_STATE_e eventState);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif
