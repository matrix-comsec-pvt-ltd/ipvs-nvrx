//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		SMSonHttp.c
@brief      This file provides API to send SMS on Http.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "SysTimer.h"
#include "ConfigApi.h"
#include "DebugLog.h"
#include "HttpClient.h"
#include "SMSonHttp.h"
#include "NetworkCommand.h"
#include "NetworkManager.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MOBILE_NO_SEP						','
#define HEX_VALUE_INDICATOR					'%'

// size of buffers will be multiplied by this value to store encoded data
#define	ENCODING_SIZE_MUL					3

// As defined in SAD this module need to take care of only 4 possible errors others are considered as CMD_TESTING_FAIL
#define MAX_HTTP_SMS_ERR					4

// This many Bytes must not be available to print a valid End of URL
#define URL_END_MARGIN						10
#define HTTP_SMS_REQ_TIMEOUT				20

// This refers to the valid SMS parameters, each server must have this many parameters while entering sequence.
// If any server doesn't have this many parameters Append PARAM_BLANK to make count same for all servers
#define MAX_VALID_SMS_REQ_PARAM             (MAX_SMS_REQ_PARAM - 1)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
	URL_INFORMATIVE_DATA,	// This type of data may contain '@', '&', etc..
    URL_USER_DATA_TYPE1,    // This type of data must contain only valid URL char except 'space'
    URL_USER_DATA_TYPE2,    // This type of data must allow only & and =
	MAX_WEB_URL_DATA_TYPE
}WEB_URL_DATA_TYPE_e;

typedef enum
{
	HTTP_SMS_SEND_SMS,
	HTTP_SMS_CHK_BALANCE,
	MAX_HTTP_SMS_REQ
}HTTP_SMS_REQ_e;

typedef enum
{
	PARAM_USER_NAME,
	PARAM_PASSWORD,
	PARAM_MOBLIE_NUMBER,
	PARAM_MESSAGE,
	PARAM_SENDOR_ID,
	PARAM_IS_MSG_FLASH,
	PARAM_BLANK,
	MAX_SMS_REQ_PARAM
}SMS_REQ_PARAM_e;

typedef struct
{
    UINT16      httpPort;
    CHARPTR     webAddr;
    CHARPTR     queryString[MAX_HTTP_SMS_REQ];
    CHARPTR     successMsg;
    CHARPTR     blncRespFrmt;
    CHARPTR     errMsg[MAX_HTTP_SMS_ERR];
    UINT8       sendAsFlashValues[2];
    UINT8       paramSeq[MAX_VALID_SMS_REQ_PARAM];

}SMS_SERVER_PARAM_t;

typedef struct
{
    BOOL                chkBlncStatus;
    SMS_SERVER_e        smsServer;
    CLIENT_CB_TYPE_e    callback;
    INT32               connFd;
    INT32               balance;
    NET_CMD_STATUS_e    cmdRespCode;
    pthread_mutex_t     chkBlncLock;

}CHK_BLNC_PARAM_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const SMS_SERVER_PARAM_t smsServerParam[MAX_SMS_SERVER] =
{
	{
        .httpPort	 =  DFLT_HTTP_PORT,
		.webAddr	 =  "www.smsgatewaycenter.com",
		.queryString =
        {				// Send SMS URL template
						"/library/send_sms_2.php?"
						"UserName=%s&"
						"Password=%s&"
						"Type=Bulk&"
						"To=%s&"
						"Mask=%s&"
						"Message=%s&",

						// Check Balance URL template
						"/library/checkbalance.php?Username=%s&Password=%s"
		},

		.successMsg	 =  "SUCCESS",
		.errMsg		 =
		{
						"USERNAME OR PASSWORD IS INVALID",
						"USER ACCOUNT HAS BEEN EXPIRED",
						"NOT HAVING ENOUGH BALANCE TO SEND SMS",	// format to extract Insufficient credit is not available
						"MOBILE NOS ENTERED ARE NOT IN VALID FORMAT"
		},

		.paramSeq	 =
		{
						PARAM_USER_NAME,
						PARAM_PASSWORD,
						PARAM_MOBLIE_NUMBER,
						PARAM_SENDOR_ID,
						PARAM_MESSAGE,
						PARAM_BLANK
		},

		.blncRespFrmt = "Balance : %d ",
		.sendAsFlashValues = {0},
	},

	{
        .httpPort	 =  DFLT_HTTPS_PORT,
        .webAddr	 =  "api.smslane.com",
		.queryString =
		{
						// Send SMS URL template
                        "/api/v2/SendSMS?"
                        "ClientId=%s&"
                        "ApiKey=%s&"
                        "MobileNumbers=%s&"
                        "SenderId=%s&"
                        "Message=%s&"
                        "Is_Flash=%d",

						// Check Balance URL template not Available
                        "/api/v2/Balance?ClientId=%s&ApiKey=%s"
		},

        .successMsg	 =  "{\"ErrorCode\":0",
		.errMsg		 =
		{
                        "\"ErrorCode\":7,",
                        "\"ErrorCode\":30,",
                        "\"ErrorCode\":35,",
                        "\"ErrorCode\":13,"
		},

		.paramSeq	 =
		{
						PARAM_USER_NAME,
						PARAM_PASSWORD,
						PARAM_MOBLIE_NUMBER,
						PARAM_SENDOR_ID,
						PARAM_MESSAGE,
						PARAM_IS_MSG_FLASH,
		},

        .blncRespFrmt = "\"CREDIT",
		.sendAsFlashValues = {0, 1},
	},

	{
        .httpPort	 =  DFLT_HTTP_PORT,
		.webAddr	 =  "www.businesssms.co.in",
		.queryString =
		{
						// Send SMS URL template
						"/sms.aspx?"
						"ID=%s&"
						"Pwd=%s&"
						"PhNo=%s&"
						"Text=%s&"
						"SMSType=%d&",

						// Check Balance URL template
						"/sms.aspx?ID=%s&Pwd=%s",
		},

		.successMsg	 =  "Message Submitted",
		.errMsg		 =
		{
						"Authentication Failed",
						"Your Account is not active",
						"Insufficient Credit",
						""	// format to extract invalid mobile number is not available
		},

		.paramSeq	 =
		{
						PARAM_USER_NAME,
						PARAM_PASSWORD,
						PARAM_MOBLIE_NUMBER,
						PARAM_MESSAGE,
						PARAM_IS_MSG_FLASH,
						PARAM_BLANK
		},

		.blncRespFrmt = "%d",
		.sendAsFlashValues = {0, 1},
	},

	{
		.httpPort	 = 5567,
		.webAddr	 =  "bulksms.vsms.net",
		.queryString =
		{
						// Send SMS URL template
						"/eapi/submission/send_sms/2/2.0?"
						"username=%s&"
						"password=%s&"
						"message=%s&"
						"msisdn=%s&"
						"msg_class=%d",

						// Check Balance URL template
						"/eapi/user/get_credits/1/1.1?username=%s&password=%s",
		},

		.successMsg	 =  "0|IN_PROGRESS",
		.errMsg		 =
		{
						"23|",
						"",
						"25|",
						""	// format to extract invalid mobile number is not available
		},

		.paramSeq	 =
		{
						PARAM_USER_NAME,
						PARAM_PASSWORD,
						PARAM_MESSAGE,
						PARAM_MOBLIE_NUMBER,
						PARAM_IS_MSG_FLASH,
						PARAM_BLANK
		},

		.blncRespFrmt = "0|%d",
        .sendAsFlashValues = {1, 0},
	},
};

static const NET_CMD_STATUS_e errCode[MAX_HTTP_SMS_ERR] =
{
	CMD_CRED_INVALID,
	CMD_SMS_ACC_EXPIRED,
	CMD_SMS_ACC_INSUFF_CREDITS,
	CMD_INVALID_MOBILE_NO,
};

static CHK_BLNC_PARAM_t 	chkBlncParam;
static SMS_ON_HTTP_CB		smsCallBack;
static SMS_SERVER_e			tmpSmsServer;
static NET_CMD_STATUS_e		smsStausCode;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static UINT16 makeWebUrl(CHARPTR outputStr, UINT16 maxBytes, CHARPTR inputStr, WEB_URL_DATA_TYPE_e urlDataType);
//-------------------------------------------------------------------------------------------------
static void PrepareFinalSmsStr(SMS_SERVER_e smsServer, CHARPTR outPtr, CHARPTR formatPtr, VOIDPTR *args, UINT8PTR paramTypePtr);
//-------------------------------------------------------------------------------------------------
static void prepareMobileNumberStr(CHARPTR num1, CHARPTR num2, CHARPTR outPut, const UINT32 outPutLength);
//-------------------------------------------------------------------------------------------------
static BOOL makeHttpReq(HTTP_SMS_REQ_e reqType, HTTP_INFO_t *httpInfo, SMS_PARAMTER_t *smsParam, SMS_CONFIG_t *smsConfig);
//-------------------------------------------------------------------------------------------------
static void sendSMSHttpCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
static void chkSMSBalanceHttpCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initialise SMS on Http module.
 */
void InitSMSonHttp(void)
{
	// Initialize check balance Parameters
    MUTEX_INIT(chkBlncParam.chkBlncLock, NULL);
	chkBlncParam.chkBlncStatus = FREE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function sends SMS through HTTP.
 * @param   smsParam
 * @param   smsConfig
 * @param   callback
 * @return  Status
 */
NET_CMD_STATUS_e SendSMSonHttp(SMS_PARAMTER_t *smsParam, SMS_CONFIG_t *smsConfig, SMS_ON_HTTP_CB callback)
{
    HTTP_INFO_t httpInfo;
    HTTP_HANDLE httpHandle;

	// Check for bad sms parameter
    if (((smsParam->mobileNumber1[0] == '\0') && (smsParam->mobileNumber2[0] == '\0')) || (smsParam->message[0] == '\0') || (callback == NULL))
    {
        return CMD_TESTING_FAIL;
    }

    if (makeHttpReq(HTTP_SMS_SEND_SMS, &httpInfo, smsParam, smsConfig) == FAIL)
    {
        return CMD_TESTING_FAIL;
    }

    // Initialise necessary variable
    smsStausCode = CMD_TESTING_FAIL;
    tmpSmsServer = smsConfig->smsServer;
    smsCallBack = callback;

    if (StartHttp(GET_REQUEST, &httpInfo, sendSMSHttpCb, 0, &httpHandle) == FAIL)
    {
        return CMD_TESTING_FAIL;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives balance of configured Http Sms service.
 * @param   smsDataPtr
 * @param   callback
 * @param   connFd
 * @return  Status
 */
NET_CMD_STATUS_e CheckHttpSMSBalance(SMS_CONFIG_t *smsDataPtr, CLIENT_CB_TYPE_e callback, INT32 connFd)
{
#if !defined(OEM_JCI)
    HTTP_INFO_t httpInfo;
    HTTP_HANDLE httpHandle;

    if (callback >= CLIENT_CB_TYPE_MAX)
    {
        return CMD_PROCESS_ERROR;
    }

    MUTEX_LOCK(chkBlncParam.chkBlncLock);
    if (chkBlncParam.chkBlncStatus == BUSY)
	{
        MUTEX_UNLOCK(chkBlncParam.chkBlncLock);
        return CMD_REQUEST_IN_PROGRESS;
    }
    chkBlncParam.chkBlncStatus = BUSY;
    MUTEX_UNLOCK(chkBlncParam.chkBlncLock);

    if (makeHttpReq(HTTP_SMS_CHK_BALANCE, &httpInfo, NULL, smsDataPtr) == FAIL)
    {
        MUTEX_LOCK(chkBlncParam.chkBlncLock);
        chkBlncParam.chkBlncStatus = FREE;
        MUTEX_UNLOCK(chkBlncParam.chkBlncLock);
        return CMD_PROCESS_ERROR;
    }

    // Store callback and connection Fd
    chkBlncParam.connFd = connFd;
    chkBlncParam.callback = callback;
    chkBlncParam.smsServer = smsDataPtr->smsServer;
    chkBlncParam.balance = 0;
    chkBlncParam.cmdRespCode = CMD_PROCESS_ERROR;

    if (StartHttp(GET_REQUEST, &httpInfo, chkSMSBalanceHttpCb, 0, &httpHandle) == FAIL)
    {
        chkBlncParam.connFd = INVALID_CONNECTION;
        chkBlncParam.callback = CLIENT_CB_TYPE_MAX;
        MUTEX_LOCK(chkBlncParam.chkBlncLock);
        chkBlncParam.chkBlncStatus = FREE;
        MUTEX_UNLOCK(chkBlncParam.chkBlncLock);
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
#else
    return CMD_PROCESS_ERROR;
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function encodes input url and stores it in outputStr.
 * @param   outputStr
 * @param   maxBytes
 * @param   inputStr
 * @param   urlDataType
 * @return  Written string length
 */
static UINT16 makeWebUrl(CHARPTR outputStr, UINT16 maxBytes, CHARPTR inputStr, WEB_URL_DATA_TYPE_e urlDataType)
{
    CHARPTR		startOfOutput = outputStr;	// reference to the beginning of output string
    BOOL		isValidChar;

    while((*inputStr) != '\0')
	{
		// write only if more than 4 bytes remaining in output
		// 1 Byte - '%'
		// 2 Byte - HEX Value
		// 1 Byte - NULL Termination
        if (((outputStr - startOfOutput) + 4) >= maxBytes)
		{
            break;
        }

        // Validate URL Char for given URL DATA Type
        switch(urlDataType)
        {
            case URL_USER_DATA_TYPE2:
            {
                if ((*inputStr == '&') || (*inputStr == '='))
                {
                    isValidChar = TRUE;
                    break;
                }
            }

            /* FALL THROUGH */
            case URL_USER_DATA_TYPE1:
            {
                // Valid Url character as per RFC-3986
                // (A~Z) (a~z)	(0~9) (-) (_) (.) (~)
                if (((*inputStr >= 'A') && (*inputStr <= 'Z')) || ((*inputStr >= 'a') && (*inputStr <= 'z'))
                        || ((*inputStr >= '0') && (*inputStr <= '9')) || (*inputStr == '-') || (*inputStr == '_')
                        || (*inputStr == '.') || (*inputStr == '~'))
                {
                    isValidChar = TRUE;
                }
                else
                {
                    isValidChar = FALSE;
                }
            }
            break;

            case URL_INFORMATIVE_DATA:
            {
                // only space is taken care, Leave else on user!!
                isValidChar = (*inputStr == ' ') ? FALSE : TRUE;
            }
            break;

            default:
            {
                isValidChar = FALSE;
            }
            break;
        }

        if (isValidChar == TRUE)
        {
            *outputStr = *inputStr;
            outputStr++;
        }
        else
        {
            *outputStr = HEX_VALUE_INDICATOR;
            outputStr++;
            outputStr += snprintf(outputStr, 3, "%02x", (*inputStr));
        }
        inputStr++;
	}
	(*outputStr) = '\0';

	return (outputStr - startOfOutput);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function works like sprintf.
 * @param   smsServer
 * @param   outPtr
 * @param   formatPtr
 * @param   args
 * @param   paramTypePtr
 */
static void PrepareFinalSmsStr(SMS_SERVER_e smsServer, CHARPTR outPtr, CHARPTR formatPtr, VOIDPTR * args, UINT8PTR paramTypePtr)
{
	UINT8					argIndex = 0;
	UINT16					strSize = 0, bytesWritten = 0;
	CHARPTR 				tmpPtr;		// Pointing to '%' in format string
	WEB_URL_DATA_TYPE_e		urlDataType;

	strSize = (MAX_RELATIVE_URL_WIDTH - 1);

    while((tmpPtr = strchr(formatPtr, '%')) != NULL)
	{
		strncpy(outPtr, formatPtr, (tmpPtr - formatPtr));

		outPtr += (tmpPtr - formatPtr);
		strSize -= (tmpPtr - formatPtr);

		// Increment format Pointer to Next to Next Byte of '%' char
		formatPtr = (tmpPtr + 2);

        // Decide Type of URL Data based on PARAM Type (Patch added for smslane)
		if (paramTypePtr[argIndex] == PARAM_MESSAGE)
		{
            urlDataType = (smsServer == SMS_SERVER_SMSLANE) ? URL_USER_DATA_TYPE2 : URL_USER_DATA_TYPE1;
		}
        else if ((smsServer == SMS_SERVER_SMSLANE) && (paramTypePtr[argIndex] == PARAM_PASSWORD))
        {
            urlDataType = URL_USER_DATA_TYPE1;
        }
		else
		{
			urlDataType = URL_INFORMATIVE_DATA;
		}

		// get data-Type character
		switch(tmpPtr[1])
		{
            case 's':
            {
                // Encode URL for given urlDataType & store it in OUTPUT string
                bytesWritten = makeWebUrl(outPtr, strSize - URL_END_MARGIN, (CHARPTR)args[argIndex], urlDataType);
                outPtr += bytesWritten;
                strSize -= bytesWritten;
                argIndex++;
            }
            break;

            case 'd':
            {
                // NOTE : it is assumed that this value will no more than one byte
                *outPtr = ((*(UINT8PTR)args[argIndex]) + '0');
                outPtr++;
                strSize--;
                argIndex++;
            }
            break;

            default:
            {
                /* Nothing to do */
            }
            break;
		}
	}

	// print remaining data
	if (*formatPtr != '\0')
	{
        snprintf(outPtr, strlen(formatPtr)+1, "%s", formatPtr);
		outPtr += strlen(formatPtr);
	}
	*outPtr = '\0';
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function prepares mobile number string for sending SMS on HTTP. It removes '+'
 *          character from string if exists. and put a ',' between two mobile number.
 * @param   num1
 * @param   num2
 * @param   outPut
 * @param   outPutLength
 */
static void prepareMobileNumberStr(CHARPTR num1, CHARPTR num2, CHARPTR outPut, const UINT32 outPutLength)
{
	UINT8	bytesStored = 0;

    // for bulk-sms format is  919724565459,91xxxxxxxx. there shouldn't be '+' in the string
    if(num1[0] != '\0')
	{
		if (num1[0] == '+')
		{
			num1++;
		}

        bytesStored += snprintf(outPut, outPutLength, "%s", num1);
        if(bytesStored > outPutLength)
        {
            EPRINT(SMS_NOTIFY, "length is greater than buffer: [outLen=%d]", bytesStored);
            bytesStored = outPutLength;
        }
	}

    if(num2[0] != '\0')
	{
		if (num2[0] == '+')
		{
			num2++;
		}

        // If valid number is already written in string separate two numbers by MOBILE_NO_SEP
		if (bytesStored > 0)
		{
            bytesStored += snprintf((outPut + bytesStored), outPutLength - bytesStored, "%c%s", MOBILE_NO_SEP, num2);
		}
		else
		{
            bytesStored += snprintf(outPut, outPutLength, "%s", num2);
		}
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function constructs http request based on sms configuration.
 * @param   reqType
 * @param   httpInfo
 * @param   smsParam
 * @param   smsConfig
 * @return  SUCCESS / FAIL
 */
static BOOL makeHttpReq(HTTP_SMS_REQ_e reqType, HTTP_INFO_t * httpInfo, SMS_PARAMTER_t * smsParam, SMS_CONFIG_t * smsConfig)
{
    UINT8   loop;
    CHAR    mobileNumberStr[(MAX_MOBILE_NUMBER_WIDTH * 2) + sizeof(MOBILE_NO_SEP) + 1];
    VOIDPTR paramValuePtr[MAX_SMS_REQ_PARAM];
    VOIDPTR paramSeqPtr[MAX_VALID_SMS_REQ_PARAM];

    if ((smsConfig->smsServer >= MAX_SMS_SERVER) || (smsConfig->userName[0] == '\0') || (smsConfig->password[0] == '\0'))
	{
        return FAIL;
    }

    if(reqType == HTTP_SMS_SEND_SMS)
    {
        // Prepare Mobile Number string in required format
        prepareMobileNumberStr(smsParam->mobileNumber1, smsParam->mobileNumber2, mobileNumberStr, sizeof(mobileNumberStr));

        // assign the parameter value pointer
        paramValuePtr[PARAM_BLANK] = "";
        paramValuePtr[PARAM_USER_NAME] = (VOIDPTR)smsConfig->userName;
        paramValuePtr[PARAM_PASSWORD] = (VOIDPTR)smsConfig->password;
        paramValuePtr[PARAM_SENDOR_ID] = (VOIDPTR)smsConfig->sendorId;
        paramValuePtr[PARAM_MOBLIE_NUMBER] = (VOIDPTR)mobileNumberStr;
        paramValuePtr[PARAM_MESSAGE] = (VOIDPTR)smsParam->message;
        paramValuePtr[PARAM_IS_MSG_FLASH] = (VOIDPTR)&smsServerParam[smsConfig->smsServer].sendAsFlashValues[smsConfig->sendAsFlash];

        // Align Parameters in Sequence as stored in Table
        for(loop = 0; loop < MAX_VALID_SMS_REQ_PARAM; loop++)
        {
            paramSeqPtr[loop] = paramValuePtr[smsServerParam [smsConfig->smsServer].paramSeq[loop]];
        }
    }
    else
    {
        if(smsServerParam[smsConfig->smsServer].queryString[HTTP_SMS_CHK_BALANCE][0] == '\0')
        {
            return FAIL;
        }

        // Observing simplicity of Check Balance Url we don't need to deep into maintaining sequence of parameters,
        // however we can maintain & merge this kind of information in implementation if required in future.
        paramSeqPtr[0] = (VOIDPTR)smsConfig->userName;
        paramSeqPtr[1] = (VOIDPTR)smsConfig->password;
    }

    // fill params common for both request type
    snprintf(httpInfo->ipAddress, MAX_CAMERA_ADDRESS_WIDTH, "%s", smsServerParam[smsConfig->smsServer].webAddr);
    httpInfo->maxConnTime = HTTP_SMS_REQ_TIMEOUT;
    httpInfo->maxFrameTime = HTTP_SMS_REQ_TIMEOUT;
    httpInfo->authMethod = AUTH_TYPE_ANY;
    httpInfo->httpUsrPwd.username[0] = '\0';
    httpInfo->httpUsrPwd.password[0] = '\0';
    httpInfo->port = smsServerParam[smsConfig->smsServer].httpPort;
    httpInfo->userAgent = CURL_USER_AGENT;
    httpInfo->interface = MAX_HTTP_INTERFACE;

    // print parameter in string in same order mentioned in the template
    PrepareFinalSmsStr(smsConfig->smsServer, httpInfo->relativeUrl, smsServerParam[smsConfig->smsServer].queryString[reqType],
            paramSeqPtr, (UINT8PTR)smsServerParam[smsConfig->smsServer].paramSeq);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function parse the response data, and gives appropriate response client.
 * @param   httpHandle
 * @param   dataInfo
 */
static void sendSMSHttpCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo)
{
	switch(dataInfo->httpResponse)
	{
        case HTTP_SUCCESS:
        {
            // check if response have some data to parse/ validate
            if ((dataInfo->storagePtr == NULL) || (dataInfo->frameSize == 0))
            {
                // As there's no data in response, consider it as a SUCCESS
                smsStausCode = CMD_SUCCESS;
                DPRINT(SMS_NOTIFY, "sms sent successfully but blank response rcvd");
                break;
            }

            // If the Success Message Found in response we can make out that Message has been transmitted successfully
            if (strncmp((CHARPTR)dataInfo->storagePtr, smsServerParam[tmpSmsServer].successMsg, strlen(smsServerParam[tmpSmsServer].successMsg)) == STATUS_OK)
            {
                smsStausCode = CMD_SUCCESS;
                break;
            }

            UINT8 errIndx;
            for(errIndx = 0; errIndx < MAX_HTTP_SMS_ERR; errIndx++)
            {
                if (smsServerParam[tmpSmsServer].errMsg[errIndx][0] == '\0')
                {
                    continue;
                }

                if (strstr((CHARPTR)dataInfo->storagePtr, smsServerParam[tmpSmsServer].errMsg[errIndx]) != NULL)
                {
                    smsStausCode = errCode[errIndx];
                    break;
                }
            }
            EPRINT(SMS_NOTIFY, "fail to send sms: [status=%d], [errMsg=%s]", smsStausCode, (CHARPTR)dataInfo->storagePtr);
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            if (smsCallBack != NULL)
            {
                smsCallBack(smsStausCode);
                smsCallBack = NULL;
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function parse the response data, and gives appropriate response client.
 * @param   httpHandle
 * @param   dataInfo
 */
static void chkSMSBalanceHttpCb(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t * dataInfo)
{
	switch(dataInfo->httpResponse)
	{
        case HTTP_SUCCESS:
        {
            // check if response have some data to parse/ validate
            if ((dataInfo->storagePtr == NULL) || (dataInfo->frameSize == 0))
            {
                break;
            }

            if (chkBlncParam.smsServer == SMS_SERVER_SMSLANE)
            {
                CHAR *pStrSearch = strstr((CHARPTR)dataInfo->storagePtr, smsServerParam[chkBlncParam.smsServer].blncRespFrmt);
                if (pStrSearch != NULL)
                {
                    pStrSearch += strlen(smsServerParam[chkBlncParam.smsServer].blncRespFrmt);
                    chkBlncParam.balance = atoi(pStrSearch);
                    chkBlncParam.cmdRespCode = CMD_SUCCESS;
                }
            }
            else
            {
                // Try to read balance in the format as mentioned in the template
                if (sscanf((CHARPTR)dataInfo->storagePtr, smsServerParam[chkBlncParam.smsServer].blncRespFrmt, &chkBlncParam.balance) > 0)
                {
                    chkBlncParam.cmdRespCode = CMD_SUCCESS;
                }
            }
        }
        break;

        case HTTP_CLOSE_ON_SUCCESS:
        case HTTP_CLOSE_ON_ERROR:
        {
            if (chkBlncParam.callback != CLIENT_CB_TYPE_MAX)
            {
                CHAR replyMsg[256];

                UINT16 outLen = snprintf(replyMsg, sizeof(replyMsg), "%c%s%c%02d%c", SOM, headerReq[RPL_CMD], FSP, chkBlncParam.cmdRespCode, FSP);

                // Add balance only if status is CMD_SUCCESS
                if(chkBlncParam.cmdRespCode == CMD_SUCCESS)
                {
                    outLen += snprintf((replyMsg + outLen), sizeof(replyMsg) - outLen, "%c%d%c%d%c%c", SOI, 1, FSP, chkBlncParam.balance, FSP, EOI);
                }

                outLen += snprintf((replyMsg + outLen), sizeof(replyMsg) - outLen, "%c", EOM);
                sendCmdCb[chkBlncParam.callback](chkBlncParam.connFd, (UINT8PTR)replyMsg, outLen, MESSAGE_REPLY_TIMEOUT);
                closeConnCb[chkBlncParam.callback](&chkBlncParam.connFd);
                chkBlncParam.callback = CLIENT_CB_TYPE_MAX;
            }

            if (chkBlncParam.cmdRespCode == CMD_SUCCESS)
            {
                DPRINT(SMS_NOTIFY, "sms account balance: [credit=%d]", chkBlncParam.balance);
            }
            else
            {
                EPRINT(SMS_NOTIFY, "fail to check balance");
            }

            MUTEX_LOCK(chkBlncParam.chkBlncLock);
            chkBlncParam.chkBlncStatus = FREE;
            MUTEX_UNLOCK(chkBlncParam.chkBlncLock);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
	}
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
