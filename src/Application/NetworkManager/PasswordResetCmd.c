//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		PasswordResetCmd.c
@brief      This file manages password reset commands and also provides APIs associated with it
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "PasswordResetCmd.h"
#include "Utils.h"
#include "NetworkManager.h"
#include "NetworkCommand.h"
#include "DebugLog.h"
#include "SmtpClient.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define PWD_RST_EMAIL_SUBJECT_STR   "Verification OTP"
#define PWD_RST_EMAIL_MSG_STR       "The OTP for resetting password is %06d for %s. Valid for 2 minutes only."

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL parsePasswordResetCmd(CHARPTR cmdStr, PWD_RST_CMD_TYPE_e *pCmdValue);
//-------------------------------------------------------------------------------------------------
static void sendPasswordResetResp(INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType, NET_CMD_STATUS_e cmdResp, UINT32 payload);
//-------------------------------------------------------------------------------------------------
static void passwordResetCmdRespCb(NET_CMD_STATUS_e cmdResp, INT32 clientSocket, BOOL closeConnF);
//-------------------------------------------------------------------------------------------------
static void clearPasswordResetSessionCmd(CHARPTR *pCmdStr, UINT8 sessionIdx, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
static void getPasswordResetOtpCmd(CHARPTR *pCmdStr, UINT8 sessionIdx, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
static void verifyPasswordResetOtpCmd(CHARPTR *pCmdStr, UINT8 sessionIdx, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
static void verifyPasswordResetQueAnsCmd(CHARPTR *pCmdStr, UINT8 sessionIdx, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
static void setNewPasswordCmd(CHARPTR *pCmdStr, UINT8 sessionIdx, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static CHAR replyMsg[CLIENT_CB_TYPE_MAX][MAX_REPLY_SZ];

static const CHARPTR pwdRstCmdStr[MAX_PWD_RST_CMD] =
{
    "REQ_PWD_RST_SESSION",
    "CLR_PWD_RST_SESSION",
    "GET_PWD_RST_OTP",
    "VERIFY_PWD_RST_OTP",
    "VERIFY_PWD_RST_QA",
    "SET_NEW_PWD"
};

static void (*pwdResetCommandFuncPtr[MAX_PWD_RST_CMD])(CHARPTR *pCmdStr, UINT8 sessionIndex, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType) =
{
    NULL,                               /* REQ_PWD_RST_SESSION */
    clearPasswordResetSessionCmd,       /* CLR_PWD_RST_SESSION */
    getPasswordResetOtpCmd,             /* GET_PWD_RST_OTP */
    verifyPasswordResetOtpCmd,          /* VERIFY_PWD_RST_OTP */
    verifyPasswordResetQueAnsCmd,       /* VERIFY_PWD_RST_QA */
    setNewPasswordCmd                   /* SET_NEW_PWD */
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Process password reset command & accordingly assigns function to it
 * @param   pCmdStr
 * @param   clientAddr - Client socket address info
 * @param   clientSockFd
 * @param   clientCbType
 * @return  SUCCESS/FAIL
 */
BOOL ProcessPasswordResetCmd(CHARPTR *pCmdStr, SOCK_ADDR_INFO_u *clientAddr, INT32 clientSockFd, CLIENT_CB_TYPE_e clientCbType)
{
    CHAR                cmdTypeStr[MAX_COMMAND_ARG_LEN];
    UINT8               sessionIndex = PWD_RST_SESSION_MAX;
    PWD_RST_CMD_TYPE_e  cmdType;
    UINT64              sessionId;

    /* Parse to get Session ID */
    if (ParseStringGetVal(pCmdStr, &sessionId, 1, FSP) == FAIL)
    {
        /* Close the socket */
        closeConnCb[clientCbType](&clientSockFd);
        EPRINT(NETWORK_MANAGER, "fail to parse session id: [msg=%s]", *pCmdStr);
        return FAIL;
    }

    /* Parse to get command type string */
    if (ParseStr(pCmdStr, FSP, cmdTypeStr, PWD_RST_CMD_STR_LEN_MAX) != SUCCESS)
    {
        /* Close the socket */
        closeConnCb[clientCbType](&clientSockFd);
        EPRINT(NETWORK_MANAGER, "fail to parse pswd rst command type str: [msg=%s]", *pCmdStr);
        return FAIL;
    }

    /* get commandtype enum from string */
    if (parsePasswordResetCmd(cmdTypeStr, &cmdType) != SUCCESS)
    {
        /* Send fail response to the client */
        sendPasswordResetResp(clientSockFd, clientCbType, CMD_INVALID_MESSAGE, 0);
        EPRINT(NETWORK_MANAGER, "invld pwd rst command type: [cmdTypeStr=%s]", cmdTypeStr);
        return FAIL;
    }

    /* Skip session id validation for session allocation command */
    if (cmdType == REQ_PWD_RST_SESSION)
    {
        CHAR ipAddrStr[IPV6_ADDR_LEN_MAX] = "";

        /* Get IP address in string format from socket address */
        GetIpAddrStrFromSockAddr(clientAddr, ipAddrStr, sizeof(ipAddrStr));
        DPRINT(NETWORK_MANAGER, "[cmd=%s], [fd=%d], [ip=%s]", cmdTypeStr, clientSockFd, ipAddrStr);

        /* Get IP address in network format from socket address */
        AllocPasswordResetSession(pCmdStr, clientAddr, clientSockFd, clientCbType);

        /* Close the socket */
        closeConnCb[clientCbType](&clientSockFd);
        return SUCCESS;
    }

    /* Validate session id */
    if (IsPasswordResetSessionValid(sessionId, &sessionIndex) == FALSE)
    {
        /* Send fail response to the client */
        sendPasswordResetResp(clientSockFd, clientCbType, CMD_SESSION_EXPIRED, 0);
        EPRINT(NETWORK_MANAGER, "invld session-id: [cmdTypeStr=%s], [sessionId=%llu]", cmdTypeStr, sessionId);
        return FAIL;
    }

    /* Client Validated */
    ReloadPasswordResetExpTmr(sessionIndex);
    DPRINT(NETWORK_MANAGER, "[cmd=%s], [fd=%d], [sessionIdx=%d]", cmdTypeStr, clientSockFd, sessionIndex);

    /* Further Action & Response will done from associated command function */
    (*pwdResetCommandFuncPtr[cmdType])(pCmdStr, sessionIndex, clientSockFd, clientCbType);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Convert password reset command string to enum
 * @param   pCmdStr
 * @param   pCmdValue
 * @return
 */
static BOOL parsePasswordResetCmd(CHARPTR cmdStr, PWD_RST_CMD_TYPE_e *pCmdValue)
{
    *pCmdValue = ConvertStringToIndex(cmdStr, pwdRstCmdStr, MAX_PWD_RST_CMD);
    if ((*pCmdValue >= REQ_PWD_RST_SESSION) && (*pCmdValue < MAX_PWD_RST_CMD))
    {
        return SUCCESS;
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare response message and send to the client
 * @param   clientSocket
 * @param   clientCbType
 * @param   cmdResp
 * @param   payload
 */
static void sendPasswordResetResp(INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType, NET_CMD_STATUS_e cmdResp, UINT32 payload)
{
    UINT32  outLen;
    CHARPTR respStringPtr = &replyMsg[clientCbType][0];

    switch(cmdResp)
    {
        case CMD_MIN_PASSWORD_CHAR_REQUIRED:
        case CMD_USER_ACCOUNT_LOCK:
        case CMD_MISMATCH_SECURITY_ANSWERS:
        case CMD_MISMATCH_OTP:
        {
            /* Prepare message with one payload value */
            outLen = snprintf(respStringPtr, MAX_REPLY_SZ, "%c%s%c%d%c%d%c%c", SOM, headerReq[RPL_PWD], FSP, cmdResp, FSP, payload, FSP, EOM);
        }
        break;

        default:
        {
            /* Prepare message only with response code */
            outLen = snprintf(respStringPtr, MAX_REPLY_SZ, "%c%s%c%d%c%c", SOM, headerReq[RPL_PWD], FSP, cmdResp, FSP, EOM);
        }
        break;
    }

    /* Send response to the client */
    sendCmdCb[clientCbType](clientSocket, (UINT8PTR)respStringPtr, outLen, MESSAGE_REPLY_TIMEOUT);

    /* Close the socket */
    closeConnCb[CLIENT_CB_TYPE_NATIVE](&clientSocket);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send command response to native type client
 * @param   status
 * @param   connFd
 * @param   closeConnF
 */
static void passwordResetCmdRespCb(NET_CMD_STATUS_e cmdResp, INT32 clientSocket, BOOL closeConnF)
{
    /* Send response to client */
    sendPasswordResetResp(clientSocket, CLIENT_CB_TYPE_NATIVE, cmdResp, 0);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function clears password reset session at index pointed by session Index
 * @param   pCmdStr
 * @param   sessionIdx
 * @param   clientSocket
 * @param   clientCbType
 */
static void clearPasswordResetSessionCmd(CHARPTR *pCmdStr, UINT8 sessionIdx, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType)
{
    /* Dealloc Password Reset Session */
    DeallocPasswordResetSession(sessionIdx);

    /* Send success response to the client */
    sendPasswordResetResp(clientSocket, clientCbType, CMD_SUCCESS, 0);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function generates new otp and send it to configured email address
 * @param   pCmdStr
 * @param   sessionIdx
 * @param   clientSocket
 * @param   clientCbType
 */
static void getPasswordResetOtpCmd(CHARPTR *pCmdStr, UINT8 sessionIdx, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType)
{    
    SMTP_CONFIG_t               smtpCfg;
    NET_CMD_STATUS_e            cmdResp = CMD_SUCCESS;
    PASSWORD_RECOVERY_CONFIG_t  pwdRecoveryCfg;
    EMAIL_PARAMETER_t           emailData;
    USER_ACCOUNT_CONFIG_t       userAccCfg;

    do
    {
        /* Get Email Client config & Validate */
        ReadSmtpConfig(&smtpCfg);

        if ((smtpCfg.smtp == DISABLE) || (smtpCfg.server[0] == '\0') || (smtpCfg.senderAddress[0] == '\0')
            || (smtpCfg.username[0] == '\0') || (smtpCfg.password[0] == '\0'))
        {
            DPRINT(NETWORK_MANAGER, "mail server not configured");
            cmdResp = CMD_EMAIL_SERVICE_DISABLED;
            break;
        }

        /* Get user's email address */
        ReadSinglePasswordRecoveryConfig(UserIndexFrmPwdRstSession(sessionIdx), &pwdRecoveryCfg);
        if (pwdRecoveryCfg.emailId[0] == '\0')
        {
            cmdResp = CMD_MISSING_VERIFICATION_DETAILS;
            break;
        }        

        /* Generate password reset OTP and Prepare Email Data */
        ReadSingleUserAccountConfig(UserIndexFrmPwdRstSession(sessionIdx), &userAccCfg);
        snprintf(emailData.emailAddress, PWD_RECOVERY_EMAIL_ID_LEN_MAX, "%s", pwdRecoveryCfg.emailId);
        snprintf(emailData.subject, MAX_EMAIL_SUBJECT_WIDTH, PWD_RST_EMAIL_SUBJECT_STR);
        snprintf(emailData.message, MAX_EMAIL_MESSAGE_WIDTH, PWD_RST_EMAIL_MSG_STR, GeneratePasswordResetOtp(sessionIdx), userAccCfg.username);

        cmdResp = SendOtpEmail(&smtpCfg, &emailData, passwordResetCmdRespCb, clientSocket);
        if(cmdResp == CMD_SUCCESS)
        {
            return;
        }

    }while(0);

    /* Send response to the client */
    sendPasswordResetResp(clientSocket, clientCbType, cmdResp, 0);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Verifies received otp and sends response accordingly
 * @param   pCmdStr
 * @param   sessionIdx
 * @param   clientSocket
 * @param   clientCbType
 */
static void verifyPasswordResetOtpCmd(CHARPTR *pCmdStr, UINT8 sessionIdx, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType)
{
    UINT64              otp6Digit;
    UINT32              accountLockDuration;
    UINT8               remainingAttempts;
    UINT32              payload = 0;
    NET_CMD_STATUS_e    cmdResp = CMD_SUCCESS;

    do
    {
        /* Get OTP from payload */
        if (ParseStringGetVal(pCmdStr, &otp6Digit, 1, FSP) == FAIL)
        {
            EPRINT(NETWORK_MANAGER, "fail to parse otp: [sessionIdx=%d], [clientCbType=%d]", sessionIdx, clientCbType);
            cmdResp = CMD_INVALID_SYNTAX;
            break;
        }

        if (otp6Digit >= 1000000L)
        {
            EPRINT(NETWORK_MANAGER, "invld otp size: [sessionIdx=%d], [clientCbType=%d]", sessionIdx, clientCbType);
            cmdResp = CMD_PROCESS_ERROR;
            break;
        }

        /* Verify OTP */
        if (otp6Digit == GetPasswordResetOtp(sessionIdx))
        {
            DPRINT(NETWORK_MANAGER, "otp verification successful: [sessionIdx=%d], [userIdx=%d]", sessionIdx, UserIndexFrmPwdRstSession(sessionIdx));
            break;
        }

        /* Passowrd reset attempt failed */
        if (UpdateInvalidPassEntry(INVALID_ATTEMPT_TYPE_PWD_RESET, UserIndexFrmPwdRstSession(sessionIdx), &accountLockDuration, &remainingAttempts) == TRUE)
        {
            /* OTP is wrong, you exceeded maximum attemp limit. Your account is locked for some time */
            WPRINT(NETWORK_MANAGER, "otp mismatch, account locked: [lockDuration=%dmin], [sessionIdx=%d], [userIdx=%d]",
                   accountLockDuration, sessionIdx, UserIndexFrmPwdRstSession(sessionIdx));
            payload = accountLockDuration;
            cmdResp = CMD_USER_ACCOUNT_LOCK;
            break;
        }

        /* OTP is wrong, you have more trials */
        WPRINT(NETWORK_MANAGER, "otp mismatch: [remainingAttempts=%d], [sessionIdx=%d], [userIdx=%d]",
               remainingAttempts, sessionIdx, UserIndexFrmPwdRstSession(sessionIdx));
        payload = remainingAttempts;
        cmdResp = CMD_MISMATCH_OTP;

    }while(0);

    /* Send response to the client */
    sendPasswordResetResp(clientSocket, clientCbType, cmdResp, payload);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Verifies security question id and their answers sent by client
 * @param   pCmdStr
 * @param   sessionIdx
 * @param   clientSocket
 * @param   clientCbType
 */
static void verifyPasswordResetQueAnsCmd(CHARPTR *pCmdStr, UINT8 sessionIdx, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType)
{
    UINT8                       ansIdx;
    UINT32                      accountLockDuration;
    UINT8                       remainingAttempts;
    UINT32                      payload = 0;
    NET_CMD_STATUS_e            cmdResp = CMD_SUCCESS;
    CHAR                        secQueAns[PWD_RECOVERY_SECURITY_QUESTION_MAX][PWD_RECOVERY_ANSWER_LEN_MAX];
    PASSWORD_RECOVERY_CONFIG_t  pwdRecoveryCfg;

    do
    {
        /* Parse all answers from msg string */
        for (ansIdx = 0; ansIdx < PWD_RECOVERY_SECURITY_QUESTION_MAX; ansIdx++)
        {
            if (ParseStr(pCmdStr, FSP, secQueAns[ansIdx], PWD_RECOVERY_ANSWER_LEN_MAX) != SUCCESS)
            {
                EPRINT(NETWORK_MANAGER, "fail to parse security answer: [sessionIdx=%d], [clientCbType=%d]", sessionIdx, clientCbType);
                cmdResp = CMD_INVALID_SYNTAX;
                break;
            }
        }

        /* All answers parsed? */
        if (cmdResp != CMD_SUCCESS)
        {
            break;
        }

        ReadSinglePasswordRecoveryConfig(UserIndexFrmPwdRstSession(sessionIdx), &pwdRecoveryCfg);
        for (ansIdx = 0; ansIdx < PWD_RECOVERY_SECURITY_QUESTION_MAX; ansIdx++)
        {
            /* If answer mis-matched */
            if (strcasecmp(secQueAns[ansIdx], pwdRecoveryCfg.securityQuestionInfo[ansIdx].answer) != STATUS_OK)
            {
                break;
            }
        }

        /* All answers are matched */
        if (ansIdx == PWD_RECOVERY_SECURITY_QUESTION_MAX)
        {
            /* All answers are correct */
            DPRINT(NETWORK_MANAGER, "verified security answers: [sessionIdx=%d], [userIdx=%d]", sessionIdx, UserIndexFrmPwdRstSession(sessionIdx));
            break;
        }

        /* Passowrd reset attempt failed */
        if (UpdateInvalidPassEntry(INVALID_ATTEMPT_TYPE_PWD_RESET, UserIndexFrmPwdRstSession(sessionIdx), &accountLockDuration, &remainingAttempts) == TRUE)
        {
            /* Answer(s) is wrong, you exceeded maximum attemp limit. Your account is locked for some time */
            WPRINT(NETWORK_MANAGER, "answer mismatch, account locked: [lockDuration=%dmin], [sessionIdx=%d], [userIdx=%d]",
                   accountLockDuration, sessionIdx, UserIndexFrmPwdRstSession(sessionIdx));
            payload = accountLockDuration;
            cmdResp = CMD_USER_ACCOUNT_LOCK;
            break;
        }

        /* Answer(s) is wrong, you have more trials */
        WPRINT(NETWORK_MANAGER, "answer mismatch: [remainingAttempts=%d], [sessionIdx=%d], [userIdx=%d]",
               remainingAttempts, sessionIdx, UserIndexFrmPwdRstSession(sessionIdx));
        payload = remainingAttempts;
        cmdResp = CMD_MISMATCH_SECURITY_ANSWERS;

    }while(0);

    /* Send response to the client */
    sendPasswordResetResp(clientSocket, clientCbType, cmdResp, payload);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Resets login password given by client
 * @param   pCmdStr
 * @param   sessionIdx
 * @param   clientSocket
 * @param   clientCbType
 */
static void setNewPasswordCmd(CHARPTR *pCmdStr, UINT8 sessionIdx, INT32 clientSocket, CLIENT_CB_TYPE_e clientCbType)
{
    UINT8                   userIdx = UserIndexFrmPwdRstSession(sessionIdx);
    UINT8                   minCharReq = 0;
    LOGIN_POLICY_CONFIG_t   loginPolicyCfg;
    USER_ACCOUNT_CONFIG_t   userAccountConfig;
    NET_CMD_STATUS_e        cmdResp = CMD_SUCCESS;

    ReadSingleUserAccountConfig(userIdx, &userAccountConfig);

    /* Parse password from payload */
    if (ParseStr(pCmdStr, FSP, userAccountConfig.password, MAX_USER_ACCOUNT_PASSWORD_WIDTH) == FAIL)
    {
        EPRINT(NETWORK_MANAGER, "fail to parse new pwd: [sessionIdx=%d], [clientCbType=%d]", sessionIdx, clientCbType);
        cmdResp = CMD_INVALID_SYNTAX;
    }
    else
    {
        ReadLoginPolicyConfig(&loginPolicyCfg);
        cmdResp = CheckPasswordPolicy(&loginPolicyCfg, &userAccountConfig, &minCharReq);
        if(cmdResp == CMD_SUCCESS)
        {
            WriteSingleUserAccountConfig(userIdx, &userAccountConfig);
            CreateNewPasswordExpiryInfo(userIdx, &userAccountConfig, FALSE);
            RemoveInvalidPassEntry(userIdx);
            WritePwdRstConfigChnageEvent(sessionIdx, EVENT_SUCCESS);
        }
    }

    /* Send response to the client */
    sendPasswordResetResp(clientSocket, clientCbType, cmdResp, minCharReq);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
