//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		SmtpClient.c
@brief      This module provides APIs to Send Emails.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "SmtpClient.h"
#include "SysTimer.h"
#include "DebugLog.h"
#include "Queue.h"
#include "Utils.h"
#include "DateTime.h"
#include "TimeZone.h"
#include "HttpClient.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Email body tag and format related parameters */
#define CR_LF_STR                       "\r\n"
#define MAIL_ATTRIB_MIME_VERSION        "MIME-Version: 1.0"
#define MAIL_ATTRIB_MAILER              "X-Mailer: %s"
#define MAIL_ATTRIB_MSG_ID              "Message-ID: "
#define MAIL_ATTRIB_DATE                "Date: "
#define MAIL_ATTRIB_SUB                 "Subject: "
#define MAIL_ATTRIB_FROM                "From: "
#define MAIL_ATTRIB_TO                  "To: "
#define MAIL_ATTRIB_CONTENT_TYPE        "Content-Type: "
#define MAIL_ATTRIB_CONTENT_ENCODING    "Content-Transfer-Encoding: "
#define MAIL_ATTRIB_CONTENT_DISPOSITION "Content-Disposition: "
#define MAIL_ATTRIB_CHAR_SET            "charset="
#define MAIL_ATTRIB_BOUNDARY            "boundary="
#define MAIL_ATTRIB_ATTACHMENT          "attachment"
#define MAIL_ATTRIB_FILE_NAME           "filename="
#define MAIL_ATTRIB_NAME                "name="

/* Charset related parameters */
#define PLAIN_CONTENT_TYPE              "text/plain"
#define CONTENT_TYPE_MULTI_MIX          "multipart/mixed"
#define DFLT_ATTACHMENT_CONTENT_TYPE    "application/octet-stream"
#define ATTACHMENT_CONTENT_TYPE_JPG     "image/jpeg"
#define CHAR_SET_UTF8                   "utf-8"
#define CHAR_SET_USASCII                "us-ascii"
#define CHAR_ENCODING_7BIT              "7bit"
#define CHAR_ENCODING_BASE64            "base64"

/* Test email parameters */
#define TEST_EMAIL_SUBJECT              "Test mail"
#define TEST_EMAIL_BODY                 "This is a test mail sent by the Device"

/* Different parameters legnth for email */
#define MAIL_ATTRIB_MAX_BYTES           (76)
#define MAIL_BOUNDARY_MAX_LEN           (20)
#define FILE_NAME_LEN_MAX               (51)
#define ATTACH_FILE_TYPE_MAX_LEN        (100)

/* Email max size without attachment (including multi-langauge) and attachment max allowed size */
#define FIXED_EMAIL_SIZE_MAX            (3*KILO_BYTE)
#define ATTACHMENT_ALLOW_SIZE_MAX       (4*MEGA_BYTE)
#define REMOVE_ATTACHMENT_FILE(file)    if (file[0] != '\0') unlink(file);

/* Different limits defines related smtp process */
#define EMAIL_WORKER_THREAD_MAX         (1)
#define SMTP_MAX_QUEUE_SIZE             (250)
#define EMAIL_THREAD_SLEEP_USEC         (1000000)
#define EMAIL_RETRY_SLEEP_SEC           (10)
#define EMAIL_RETRY_CNT_MAX             (5)
#define EMAIL_CLIENT_THREAD_STACK_SZ    (1*MEGA_BYTE)

/* SMTP server response codes */
#define SMTP_CONNECTION_ERROR					(101)
#define SMTP_CONNNECTION_REFUSED				(111)
#define SMTP_SERVER_UNAVAILABLE					(421)
#define SMTP_RECIPIENT_SERVER_FULL				(422)
#define SMTP_RECIPIENT_SERVER_NOT_RESPONDING	(441)
#define SMTP_RECIPIENT_MAILBOX_UNAVAILABLE		(450)
#define SMTP_SERVER_STORAGE_FULL				(452)
#define SMTP_INVALID_SYNTEX						(501)
#define SMTP_BAD_SEQUENCE						(503)
#define SMTP_INVALID_PARAMETER					(504)
#define SMTP_BAD_EMAIL_ADDR_CODE1				(510)
#define SMTP_BAD_EMAIL_ADDR_CODE2				(511)
#define SMTP_DNS_ERROR							(512)
#define	SMTP_INCORRECT_EMAIL_ADDR				(513)
#define SMTP_MAIL_NOT_ACCEPTED					(521)
#define SMTP_INVALID_AUTHENTICATION				(530)
#define SMTP_APP_AUTHENTICATION					(534)
#define SMTP_AUTHENTICATION_FAILED				(535)
#define SMTP_EMAIL_NOT_EXIST					(550)
#define SMTP_RECIPIENT_MAILBOX_FULL				(552)
#define SMTP_INVALID_MAILBOX_ADDR				(553)
#define SMTP_TRANSACTION_FAILED					(554)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    QUICK_EMAIL_TYPE_TEST = 0,
    QUICK_EMAIL_TYPE_OTP,
    QUICK_EMAIL_TYPE_MAX
}QUICK_EMAIL_TYPE_e;

typedef struct
{
    CHAR toField[MAX_EMAIL_ADDRESS_WIDTH];
    CHAR subject[MAX_EMAIL_SUBJECT_WIDTH];
    CHAR body[MAX_EMAIL_MESSAGE_WIDTH];
    CHAR pAttachment[FILE_NAME_LEN_MAX];
}EMAIL_QUEUE_ENTRY_t;

typedef struct
{
    QUICK_EMAIL_TYPE_e  emailType;
	BOOL			 	runStatus;
	NW_CMD_REPLY_CB  	callback;
	INT32			 	connFd;
	pthread_mutex_t 	emailMutex;
    EMAIL_QUEUE_ENTRY_t emailPara;
	SMTP_CONFIG_t		emailConfig;
}QUICK_EMAIL_INFO_t;

typedef struct
{
    BOOL        exitGuard;
    BOOL        reloadConfig;
    pthread_t   threadId;
} EMAIL_THREAD_PARAM_t;

typedef struct
{
    CHAR    dispName[FILE_NAME_LEN_MAX];
    UINT32  fileSize;
    CHAR    contentType[ATTACH_FILE_TYPE_MAX_LEN];
} EMAIL_ATTACHMENT_INFO_t;

typedef struct
{
    UINT32  totalLength;
    UINT32  bytesRead;
    CHAR    *pEmailData;
} EMAIL_PAYLOAD_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static EMAIL_THREAD_PARAM_t emailWorkerThread[EMAIL_WORKER_THREAD_MAX];
static QUEUE_HANDLE         hEmailQueue = NULL;
static QUICK_EMAIL_INFO_t   quickEmailInfo[QUICK_EMAIL_TYPE_MAX];

//#################################################################################################
// @PROTOTYPE
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void *emailClientThread(void *threadParam);
//-------------------------------------------------------------------------------------------------
static void *quickEmailClientThread(void *threadParam);
//-------------------------------------------------------------------------------------------------
static void smtpClientQueueFullCb(VOIDPTR entry);
//-------------------------------------------------------------------------------------------------
static CURLcode performEmailRequest(const SMTP_CONFIG_t *pSmtpConfig, EMAIL_QUEUE_ENTRY_t *pQueueEntry, INT32 *pRespErrCode);
//-------------------------------------------------------------------------------------------------
static BOOL getAttachmentInfo(const CHAR *pAttachmentPath, EMAIL_ATTACHMENT_INFO_t *pAttachmentInfo);
//-------------------------------------------------------------------------------------------------
static UINT32 prepareEmailMsgData(CHAR *pEmailData, UINT32 totalLen, const SMTP_CONFIG_t *pSmtpConfig,
                                  EMAIL_QUEUE_ENTRY_t *pQueueEntry, EMAIL_ATTACHMENT_INFO_t *pAttachInfo);
//-------------------------------------------------------------------------------------------------
static BOOL isAsciiString(CHAR *pStr);
//-------------------------------------------------------------------------------------------------
static UINT32 prepareMailSubject(CHAR *pEmailBuf, UINT32 bufLen, CHAR *pSubject);
//-------------------------------------------------------------------------------------------------
static UINT32 prepareMailDate(CHAR *pEmailBuf, UINT32 bufLen);
//-------------------------------------------------------------------------------------------------
static UINT32 prepareMailBody(CHAR *pEmailBuf, UINT32 bufLen, CHAR *pBody);
//-------------------------------------------------------------------------------------------------
static UINT32 prepareMailAttachment(CHAR *pEmailBuf, UINT32 bufLen, CHAR *pAttachment, EMAIL_ATTACHMENT_INFO_t *pAttachInfo);
//-------------------------------------------------------------------------------------------------
static CURLcode sendCurlSmtpRequest(const SMTP_CONFIG_t *pSmtpConfig, EMAIL_QUEUE_ENTRY_t *pQueueEntry,
                                    EMAIL_PAYLOAD_INFO_t *pEmailPayloadInfo, INT32 *pRespErrCode);
//-------------------------------------------------------------------------------------------------
static size_t emailPayalodUploadCb(CHAR *pUploadData, size_t size, size_t nmemb, void *pUserData);
//-------------------------------------------------------------------------------------------------
static BOOL writeEmailErrInEventLog(EMAIL_QUEUE_ENTRY_t *pQueueEntry, CURLcode curlCode, INT32 respErrCode, UINT8 *pEmailRetryCnt);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getQuickEmailCmdRespCode(QUICK_EMAIL_TYPE_e emailType, CURLcode curlCode, INT32 respErrCode);
//-------------------------------------------------------------------------------------------------
static const CHAR *getQuickEmailTypeStr(QUICK_EMAIL_TYPE_e emailType);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTION DEFINATION
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initialize smtp client with email queue and creates thread for sending an email.
 * @return  SUCCESS/FAIL
 */
BOOL InitSmtpClient(void)
{
    UINT8           threadCnt;
    UINT8           quickEmailType;
    QUEUE_INIT_t    emailQueue;

    /* Create a queue for email parameters */
    QUEUE_PARAM_INIT(emailQueue);
    emailQueue.maxNoOfMembers = SMTP_MAX_QUEUE_SIZE;
    emailQueue.sizoOfMember = sizeof(EMAIL_QUEUE_ENTRY_t);
    emailQueue.callback = smtpClientQueueFullCb;
    hEmailQueue = QueueCreate(&emailQueue);

    /* Reset quick email info */
    for (quickEmailType = 0; quickEmailType < QUICK_EMAIL_TYPE_MAX; quickEmailType++)
    {
        quickEmailInfo[quickEmailType].emailType = quickEmailType;
        quickEmailInfo[quickEmailType].runStatus = STOP;
        quickEmailInfo[quickEmailType].callback = NULL;
        quickEmailInfo[quickEmailType].connFd = INVALID_CONNECTION;
        MUTEX_INIT(quickEmailInfo[quickEmailType].emailMutex, NULL);
        memset(&quickEmailInfo[quickEmailType].emailPara, 0, sizeof(quickEmailInfo[quickEmailType].emailPara));
        memset(&quickEmailInfo[quickEmailType].emailConfig, 0, sizeof(quickEmailInfo[quickEmailType].emailConfig));
    }

    /* Reset data of thread descriptor */
    memset(emailWorkerThread, 0, sizeof(emailWorkerThread));

    /* Create worker threads to send emails */
    for (threadCnt = 0; threadCnt < EMAIL_WORKER_THREAD_MAX; threadCnt++)
    {
        /* Run thread in super loop */
        emailWorkerThread[threadCnt].exitGuard = FALSE;

        /* Create the email thread */
        if (FAIL == Utils_CreateThread(&emailWorkerThread[threadCnt].threadId, emailClientThread,
                                       &emailWorkerThread[threadCnt], JOINABLE_THREAD, EMAIL_CLIENT_THREAD_STACK_SZ))
        {
            emailWorkerThread[threadCnt].exitGuard = TRUE;
            EPRINT(EMAIL_NOTIFY, "fail to create smtp client thread: [threadCnt=%d]", threadCnt);
            return FAIL;
        }
    }

    /* Email client started */
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function frees up resources used by this module and if thread is sleep state the
 *          wake up the thread and sends terminate signal to thread.
 * @return  SUCCESS/FAIL
 */
BOOL DeinitSmtpClient(void)
{
    UINT8 threadCnt;

    /* Stop email send worker threads */
    for (threadCnt = 0; threadCnt < EMAIL_WORKER_THREAD_MAX; threadCnt++)
    {
        /* Is worker thread running? */
        if (0 == emailWorkerThread[threadCnt].threadId)
        {
            continue;
        }

        /* Signal threads to exit */
        emailWorkerThread[threadCnt].exitGuard = TRUE;

        /* Join thread to release memory */
        pthread_join(emailWorkerThread[threadCnt].threadId, NULL);
    }

    /* Reset data of thread descriptor */
    memset(emailWorkerThread, 0, sizeof(emailWorkerThread));

    /* Destroy notification queue */
    QueueDestroy(hEmailQueue);
    DPRINT(EMAIL_NOTIFY, "smtp client deinit successfully");
	return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Change in smtp configuration
 * @param newCopy
 * @param oldCopy
 */
void UpdateSmtpConfig(SMTP_CONFIG_t newCopy, SMTP_CONFIG_t *oldCopy)
{
    UINT8 threadCnt;

    /* Set config reload for all worker threads */
    for (threadCnt = 0; threadCnt < EMAIL_WORKER_THREAD_MAX; threadCnt++)
    {
        /* Is worker thread running? */
        if (0 == emailWorkerThread[threadCnt].threadId)
        {
            continue;
        }

        /* Update smtp config on config change */
        emailWorkerThread[threadCnt].reloadConfig = TRUE;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function accepts an email request from user and placed it in queue.
 * @param   pEmailParameter
 * @param   pFileAttach
 * @return  Returns success if processed else returns with error cause
 */
NET_CMD_STATUS_e ProcessEmail(EMAIL_PARAMETER_t *pEmailParam, CHARPTR pAttachment)
{
    SMTP_CONFIG_t       smtpCfg;
    EMAIL_QUEUE_ENTRY_t email;

    /* Do not send email if smtp disabled */
    ReadSmtpConfig(&smtpCfg);
    if (smtpCfg.smtp == DISABLE)
    {
        EPRINT(EMAIL_NOTIFY, "discard email: smtp disabled");
        WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Email service disabled", EVENT_FAIL);
        return CMD_SERVER_DISABLED;
    }

    /* Validate input parameter */
    if ((hEmailQueue == NULL) || (pEmailParam == NULL) || (pEmailParam->emailAddress[0] == '\0'))
    {
        EPRINT(EMAIL_NOTIFY, "null pointer found or email is blank");
        return CMD_PROCESS_ERROR;
    }

    /* Now put email with parameter in queue */
    memcpy(email.toField, pEmailParam->emailAddress, sizeof(pEmailParam->emailAddress));
    memcpy(email.subject, pEmailParam->subject, sizeof(pEmailParam->subject));
    memcpy(email.body, pEmailParam->message, sizeof(pEmailParam->message));
    snprintf(email.pAttachment, FILE_NAME_LEN_MAX, "%s", (pAttachment == NULL) ? "" : pAttachment);

    /* Add email entry in queue */
    if (QueueAddEntry(hEmailQueue, (VOIDPTR)&email) == FAIL)
    {
        EPRINT(EMAIL_NOTIFY, "discard email: fail to add in queue");
        return CMD_RESOURCE_LIMIT;
    }

    /* Email added in queue successfully */
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function accepts a test email request from user and creates a thread to send it immediately.
 * @param   pToField
 * @param   smptDataPtr
 * @param   callback
 * @param   clientFd
 * @return  Returns success if processed else returns with error cause
 */
NET_CMD_STATUS_e SendTestEmail(CHARPTR pToField, SMTP_CONFIG_t *pSmptConfig, NW_CMD_REPLY_CB callback, INT32 clientFd)
{
    pthread_t           threadId;
    QUICK_EMAIL_TYPE_e  emailType = QUICK_EMAIL_TYPE_TEST;

    /* Validate input parameter */
    if ((pToField == NULL) || (*pToField == '\0'))
    {
        DPRINT(EMAIL_NOTIFY, "null pointer found");
        return CMD_INVALID_SYNTAX;
    }

    /* Is test email already running? */
    MUTEX_LOCK(quickEmailInfo[emailType].emailMutex);
    if(quickEmailInfo[emailType].runStatus == START)
    {
        MUTEX_UNLOCK(quickEmailInfo[emailType].emailMutex);
        DPRINT(EMAIL_NOTIFY, "test mail already running");
        return CMD_TESTING_ON;
    }
    quickEmailInfo[emailType].runStatus = START;
    MUTEX_UNLOCK(quickEmailInfo[emailType].emailMutex);

    /* Store smtp config for sending test email */
    quickEmailInfo[emailType].emailConfig = *pSmptConfig;

    /* Store email parameters for sending test email */
    snprintf(quickEmailInfo[emailType].emailPara.toField, sizeof(quickEmailInfo[emailType].emailPara.toField), "%s", pToField);
    snprintf(quickEmailInfo[emailType].emailPara.subject, sizeof(quickEmailInfo[emailType].emailPara.subject), TEST_EMAIL_SUBJECT);
    snprintf(quickEmailInfo[emailType].emailPara.body, sizeof(quickEmailInfo[emailType].emailPara.body), TEST_EMAIL_BODY);
    quickEmailInfo[emailType].emailPara.pAttachment[0] = '\0';
    quickEmailInfo[emailType].callback = callback;
    quickEmailInfo[emailType].connFd = clientFd;

    /* Create test mail thread */
    if (FAIL == Utils_CreateThread(&threadId, quickEmailClientThread, &quickEmailInfo[emailType], DETACHED_THREAD, EMAIL_CLIENT_THREAD_STACK_SZ))
    {
        EPRINT(EMAIL_NOTIFY, "fail to create test mail smtp client thread");
        return CMD_TESTING_FAIL;
    }

    /* Test mail process started successfully */
    DPRINT(EMAIL_NOTIFY, "test mail process started: [server={%s}:%d], [toField=%s]",
           pSmptConfig->server, pSmptConfig->serverPort, quickEmailInfo[emailType].emailPara.toField);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function accepts a otp generation request from user and creates a thread to send it immediately.
 * @param   pSmptConfig
 * @param   pEmailParam
 * @param   callback
 * @param   clientFd
 * @return  Returns success if processed else returns with error cause
 */
NET_CMD_STATUS_e SendOtpEmail(SMTP_CONFIG_t *pSmptConfig, EMAIL_PARAMETER_t *pEmailParam, NW_CMD_REPLY_CB callback, INT32 clientFd)
{
    pthread_t           threadId;
    QUICK_EMAIL_TYPE_e  emailType = QUICK_EMAIL_TYPE_OTP;

    /* Is otp email already running? */
    MUTEX_LOCK(quickEmailInfo[emailType].emailMutex);
    if(quickEmailInfo[emailType].runStatus == START)
    {
        MUTEX_UNLOCK(quickEmailInfo[emailType].emailMutex);
        DPRINT(EMAIL_NOTIFY, "otp mail already running");
        return CMD_REQUEST_IN_PROGRESS;
    }
    quickEmailInfo[emailType].runStatus = START;
    MUTEX_UNLOCK(quickEmailInfo[emailType].emailMutex);

    /* Store smtp config for sending otp email */
    quickEmailInfo[emailType].emailConfig = *pSmptConfig;

    /* Store email parameters for sending otp email */
    snprintf(quickEmailInfo[emailType].emailPara.toField, sizeof(quickEmailInfo[emailType].emailPara.toField), "%s", pEmailParam->emailAddress);
    snprintf(quickEmailInfo[emailType].emailPara.subject, sizeof(quickEmailInfo[emailType].emailPara.subject), "%s", pEmailParam->subject);
    snprintf(quickEmailInfo[emailType].emailPara.body, sizeof(quickEmailInfo[emailType].emailPara.body), "%s", pEmailParam->message);
    quickEmailInfo[emailType].emailPara.pAttachment[0] = '\0';
    quickEmailInfo[emailType].callback = callback;
    quickEmailInfo[emailType].connFd = clientFd;

    /* Create otp mail thread */
    if (FAIL == Utils_CreateThread(&threadId, quickEmailClientThread, &quickEmailInfo[emailType], DETACHED_THREAD, EMAIL_CLIENT_THREAD_STACK_SZ))
    {
        EPRINT(EMAIL_NOTIFY, "fail to create otp mail smtp client thread");
        return CMD_TESTING_FAIL;
    }

    /* OTP mail process started successfully */
    DPRINT(EMAIL_NOTIFY, "otp mail process started: [server={%s}:%d], [toField=%s]",
           pSmptConfig->server, pSmptConfig->serverPort, quickEmailInfo[emailType].emailPara.toField);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is main routine of thread which sends email
 * @param   pthreadParam
 */
static void *emailClientThread(void *threadParam)
{
    INT32                   respErrCode;
    CURLcode                curlCode;
    UINT8                   sleepCnt;
    UINT8                   emailRetryCnt = 0;
    SMTP_CONFIG_t           smtpConfig = {0};
    EMAIL_QUEUE_ENTRY_t     *pQueueEntry = NULL;
    EMAIL_THREAD_PARAM_t    *pThreadParam = ((EMAIL_THREAD_PARAM_t*)threadParam);

    THREAD_START("EMAIL_NOTIFY");

    /* Read smtp config */
    ReadSmtpConfig(&smtpConfig);

    /* Check for therad exit signal */
    while (FALSE == pThreadParam->exitGuard)
    {
        /* Get email entry from queue only if current entry is null */
        if (NULL == pQueueEntry)
        {
            /* Get single notification entry from queue */
            pQueueEntry = QueueGetAndFreeEntry(hEmailQueue, FALSE);
            if (NULL == pQueueEntry)
            {
                /* Sleep for some time */
                usleep(EMAIL_THREAD_SLEEP_USEC);
                continue;
            }
        }

        /* Do we need to reload config? */
        if (TRUE == pThreadParam->reloadConfig)
        {
            /* Read smtp config */
            ReadSmtpConfig(&smtpConfig);
            pThreadParam->reloadConfig = FALSE;
        }

        /* Is smtp config disabled? */
        if (DISABLE == smtpConfig.smtp)
        {
            /* Remove attachment */
            REMOVE_ATTACHMENT_FILE(pQueueEntry->pAttachment);

            /* Free notification queue data pointer */
            FREE_MEMORY(pQueueEntry);
            continue;
        }

        /* Perform smtp request using curl library */
        curlCode = performEmailRequest(&smtpConfig, pQueueEntry, &respErrCode);

        /* Add failure event in event log on error */
        if (TRUE == writeEmailErrInEventLog(pQueueEntry, curlCode, respErrCode, &emailRetryCnt))
        {
            /* Remove attachment */
            REMOVE_ATTACHMENT_FILE(pQueueEntry->pAttachment);

            /* Free notification queue data pointer */
            FREE_MEMORY(pQueueEntry);
            continue;
        }

        /* Generate small sleep to reduce blockage and need to retry after timeout */
        sleepCnt = EMAIL_RETRY_SLEEP_SEC;
        while(sleepCnt)
        {
            /* We have to check exit guard after timeout */
            usleep(EMAIL_THREAD_SLEEP_USEC);
            sleepCnt--;

            /* Do we have to exit? */
            if (TRUE == pThreadParam->exitGuard)
            {
                /* Exit needed from thread */
                break;
            }
        }
    }

    /* Exit from thread */
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is main routine of quick email thread which sends actual email
 * @param   pthreadParam
 */
static void *quickEmailClientThread(void *threadParam)
{
    INT32               respErrCode;
    CURLcode            curlCode;
    NET_CMD_STATUS_e    cmdResp;
    QUICK_EMAIL_INFO_t  *pQuickEmailInfo = (QUICK_EMAIL_INFO_t*)threadParam;

    THREAD_START(getQuickEmailTypeStr(pQuickEmailInfo->emailType));

    /* Perform smtp request using curl library */
    curlCode = performEmailRequest(&pQuickEmailInfo->emailConfig, &pQuickEmailInfo->emailPara, &respErrCode);

    /* Get quick email command response code */
    cmdResp = getQuickEmailCmdRespCode(pQuickEmailInfo->emailType, curlCode, respErrCode);

    /* Is callback set for response? */
    if (pQuickEmailInfo->callback != NULL)
    {
        /* Send quick email response to client */
        pQuickEmailInfo->callback(cmdResp, pQuickEmailInfo->connFd, TRUE);
        pQuickEmailInfo->callback = NULL;
        pQuickEmailInfo->connFd = INVALID_CONNECTION;
    }

    /* Free quick email resources */
    MUTEX_LOCK(pQuickEmailInfo->emailMutex);
    pQuickEmailInfo->runStatus = STOP;
    MUTEX_UNLOCK(pQuickEmailInfo->emailMutex);

    /* Exit from thread */
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This callback is called when queue is full, and the entry is being over written
 * @param   entry
 */
static void smtpClientQueueFullCb(VOIDPTR entry)
{
    WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Email Queue Full", EVENT_FAIL);
    REMOVE_ATTACHMENT_FILE(((EMAIL_QUEUE_ENTRY_t*)entry)->pAttachment);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Perform email send request
 * @param   pSmtpConfig
 * @param   pQueueEntry
 * @param   pRespErrCode
 * @return  Returns curl easy perform response code
 */
static CURLcode performEmailRequest(const SMTP_CONFIG_t *pSmtpConfig, EMAIL_QUEUE_ENTRY_t *pQueueEntry, INT32 *pRespErrCode)
{
    CHAR                    *pEmailData = NULL;
    CURLcode                curlCode = CURLE_OK;
    UINT32                  totalLen = FIXED_EMAIL_SIZE_MAX;
    EMAIL_ATTACHMENT_INFO_t attachmentInfo;
    EMAIL_PAYLOAD_INFO_t    emailPayload;

    /* Init response code */
    *pRespErrCode = 0;

    /* Validate smtp configuration */
    if ((pSmtpConfig->server[0] == '\0') || (pSmtpConfig->serverPort == 0) || (pSmtpConfig->senderAddress[0] == '\0'))
    {
        EPRINT(EMAIL_NOTIFY, "invld smtp config: [server=%s], [port=%d], [senderAddr=%s]",
               pSmtpConfig->server, pSmtpConfig->serverPort, pSmtpConfig->senderAddress);
        *pRespErrCode = SMTP_INVALID_PARAMETER;
        return (CURLE_SEND_ERROR);
    }

    /* Validate email mandatory fields */
    if ((pQueueEntry->toField[0] == '\0') || (pQueueEntry->subject[0] == '\0'))
    {
        EPRINT(EMAIL_NOTIFY, "invld email param: [mailTo=%s], [subject=%s]", pQueueEntry->toField, pQueueEntry->subject);
        *pRespErrCode = SMTP_INVALID_PARAMETER;
        return (CURLE_SEND_ERROR);
    }

    /* Is attachment present? */
    memset(&attachmentInfo, 0, sizeof(attachmentInfo));
    if ('\0' != pQueueEntry->pAttachment[0])
    {
        /* Check size of Attachment to be sent */
        if (FAIL == getAttachmentInfo(pQueueEntry->pAttachment, &attachmentInfo))
        {
            /* Email discarded as attachment is not valid */
            *pRespErrCode = SMTP_INVALID_PARAMETER;
            return (CURLE_SEND_ERROR);
        }

        /* Add attachment length */
        totalLen = totalLen + GET_PLAIN_TO_BASE64_SIZE(attachmentInfo.fileSize)
                + ((GET_PLAIN_TO_BASE64_SIZE(attachmentInfo.fileSize)/MAIL_ATTRIB_MAX_BYTES) * (strlen(CR_LF_STR)));
    }

    /* Allocate memory for email data */
    pEmailData = (CHAR *)malloc(totalLen);
    if (NULL == pEmailData)
    {
        /* Failed to allocate memory to email data */
        EPRINT(EMAIL_NOTIFY, "fail to alloc memory");
        return (CURLE_OUT_OF_MEMORY);
    }

    /* Prepare email message data */
    totalLen = prepareEmailMsgData(pEmailData, totalLen, pSmtpConfig, pQueueEntry, &attachmentInfo);

    /* Set email payload info */
    emailPayload.bytesRead = 0;
    emailPayload.totalLength = totalLen;
    emailPayload.pEmailData = pEmailData;

    /* Send email using curl request */
    curlCode = sendCurlSmtpRequest(pSmtpConfig, pQueueEntry, &emailPayload, pRespErrCode);

    /* Free mail memory */
    free(pEmailData);

    /* Return curl response code */
    return curlCode;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get attachment information
 * @param   pAttachmentPath
 * @param   pAttachmentInfo
 * @return  SUCCESS/FAIL
 */
static BOOL getAttachmentInfo(const CHAR *pAttachmentPath, EMAIL_ATTACHMENT_INFO_t *pAttachmentInfo)
{
    INT32       fileFd;
    CHAR        *pPatternPtr = NULL;
    struct stat fileSizeInfo;

    /* Find Last "/" in File Path to Get File Name */
    pPatternPtr = strrchr(pAttachmentPath, '/');
    if ((NULL == pPatternPtr) || ('\0' == *(pPatternPtr + 1)))
    {
        EPRINT(EMAIL_NOTIFY, "invld attachment file path: [path=%s]", pAttachmentPath);
        return FAIL;
    }

    /* Jump to Next Character of "/" */
    pPatternPtr++;

    /* Set attachment display name */
    snprintf(pAttachmentInfo->dispName, sizeof(pAttachmentInfo->dispName), "%s", pPatternPtr);

    /* Open File to be Attached */
    fileFd = open(pAttachmentPath, READ_ONLY_MODE);
    if (fileFd == INVALID_FILE_FD)
    {
        EPRINT(EMAIL_NOTIFY, "fail to open attachment file: [path=%s], [err=%s]", pAttachmentPath, STR_ERR);
        return FAIL;
    }

    /* Read file info */
    if (STATUS_OK != fstat(fileFd, &fileSizeInfo))
    {
        close(fileFd);
        EPRINT(EMAIL_NOTIFY, "fail to get file info: [path=%s], [err=%s]", pAttachmentPath, STR_ERR);
        return FAIL;
    }

    /* Close the file */
    close(fileFd);

    /* Validate file size */
    if ((fileSizeInfo.st_size == 0) || ((UINT32)fileSizeInfo.st_size > ATTACHMENT_ALLOW_SIZE_MAX))
    {
        EPRINT(EMAIL_NOTIFY, "invld attachment file size: [path=%s], [size=%lld]", pAttachmentPath, (UINT64)fileSizeInfo.st_size);
        return FAIL;
    }

    /* Store attachment file size */
    pAttachmentInfo->fileSize = fileSizeInfo.st_size;

    /* Check for jpg extension */
    if (NULL != strstr(pAttachmentPath, ".jpg"))
    {
        /* Set image/jpeg content type */
        snprintf(pAttachmentInfo->contentType, sizeof(pAttachmentInfo->contentType), ATTACHMENT_CONTENT_TYPE_JPG);
    }
    else
    {
        /* Set generice content type */
        snprintf(pAttachmentInfo->contentType, sizeof(pAttachmentInfo->contentType), DFLT_ATTACHMENT_CONTENT_TYPE);
    }

    /* Got required info of attachment */
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare email message data to be sent in email
 * @param   pEmailData
 * @param   totalLen
 * @param   pSmtpConfig
 * @param   pQueueEntry
 * @param   pAttachInfo
 * @return  Number of characters written into email data
 */
static UINT32 prepareEmailMsgData(CHAR *pEmailData, UINT32 totalLen, const SMTP_CONFIG_t *pSmtpConfig,
                                  EMAIL_QUEUE_ENTRY_t *pQueueEntry, EMAIL_ATTACHMENT_INFO_t *pAttachInfo)
{
    UINT32  emailLen = 0;
    CHAR    *pPatternPtr = NULL;
    CHAR    randomStr[MAX_UUID_LEN];

    /* Add separator as per requirement */
    ReplaceCharecter(pQueueEntry->toField, ';', ',');

    /* Prepare From: Field */
    emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, MAIL_ATTRIB_FROM "%s", pSmtpConfig->senderAddress);
    emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, CR_LF_STR);

    /* Prepare To: Field */
    emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, MAIL_ATTRIB_TO "%s", pQueueEntry->toField);
    emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, CR_LF_STR);

    /* Prepare Subject: Field */
    emailLen += prepareMailSubject(&pEmailData[emailLen], totalLen - emailLen, pQueueEntry->subject);
    emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, CR_LF_STR);

    /* Prepare Date: Field */
    emailLen += prepareMailDate(&pEmailData[emailLen], totalLen - emailLen);
    emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, CR_LF_STR);

    /* Generate Uniqe Message Id For Each Mail */
    GenerateUuidStr(randomStr);
    if (NULL != (pPatternPtr = strchr(pSmtpConfig->senderAddress, '@')))
    {
        emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, MAIL_ATTRIB_MSG_ID "<%s%s>", randomStr, pPatternPtr);
    }
    else
    {
        emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, MAIL_ATTRIB_MSG_ID "<%s>", randomStr);
    }
    emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, CR_LF_STR);

    /* Add Mime-Version: 1.0 */
    emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, MAIL_ATTRIB_MIME_VERSION);
    emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, CR_LF_STR);

    /* Add X-Mailer: libcurl */
    emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, MAIL_ATTRIB_MAILER, curl_version());
    emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, CR_LF_STR);

    /* Mail Without Attachment */
    if ('\0' == pQueueEntry->pAttachment[0])
    {
        /* Prapare message body without attachment */
        emailLen += prepareMailBody(&pEmailData[emailLen], totalLen - emailLen, pQueueEntry->body);
    }
    /* Mail with Attachment */
    else
    {
        /* Generate Boundary by random number */
        GetRandomAlphaNumStr(randomStr, MAIL_BOUNDARY_MAX_LEN + 1, FALSE);

        /* Content-Type: multipart/mixed; boundary="xyz" */
        emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, MAIL_ATTRIB_CONTENT_TYPE CONTENT_TYPE_MULTI_MIX "; ");
        emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, MAIL_ATTRIB_BOUNDARY "\"%s\"", randomStr);
        emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, CR_LF_STR CR_LF_STR);

        /* Add tag of attachment --boundary */
        emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, "--%s", randomStr);
        emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, CR_LF_STR);

        /* Prapare message body with attachment */
        emailLen += prepareMailBody(&pEmailData[emailLen], totalLen - emailLen, pQueueEntry->body);

        /* Add start tag of attachment --boundary */
        emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, "--%s", randomStr);
        emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, CR_LF_STR);

        /* Content-Type: "image/jpeg"; name=Camera01-16_02_46.jpg; charset=UTF-8 */
        emailLen += prepareMailAttachment(&pEmailData[emailLen], totalLen - emailLen, pQueueEntry->pAttachment, pAttachInfo);

        /* Add end tag of attachment --boundary */
        emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, "--%s--", randomStr);
        emailLen += snprintf(&pEmailData[emailLen], totalLen - emailLen, CR_LF_STR);
    }

    /* Return number of email data characters/bytes */
    return (emailLen);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check given string is asccii or not
 * @param   pStr
 * @return  Returns TRUE if ascii string else returns FALSE
 */
static BOOL isAsciiString(CHAR *pStr)
{
    /* Value should be 0-127 */
    while (*pStr != '\0')
    {
        if (*pStr & 0x80)
        {
            /* It is not an ascii string */
            return FALSE;
        }
        pStr++;
    }

    /* It is a ascii string */
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare email subject by taking care of multi-language
 * @param   pEmailBuf
 * @param   bufLen
 * @param   pSubject
 * @return  Number of characters of subject added in email data
 */
static UINT32 prepareMailSubject(CHAR *pEmailBuf, UINT32 bufLen, CHAR *pSubject)
{
    /* Is subject string ascii string? */
    if (TRUE == isAsciiString(pSubject))
    {
        /* It is ascii string. Add subject in email as it is */
        return snprintf(pEmailBuf, bufLen, MAIL_ATTRIB_SUB "%s", pSubject);
    }
    else
    {
        UINT32  subjectLen;
        CHAR    *pSubjectB64 = EncodeBase64(pSubject, strlen(pSubject));

        /* =?<Charset>?<Encoding>?<Encoded Data>?= */
        subjectLen = snprintf(pEmailBuf, bufLen, MAIL_ATTRIB_SUB "=?%s?B?%s?=", CHAR_SET_UTF8, pSubjectB64);
        FREE_MEMORY(pSubjectB64);

        /* Return number of added characters in email data */
        return subjectLen;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare email date as per ISO format
 * @param   pEmailBuf
 * @param   bufLen
 * @return  Number of characters of date added in email data
 */
static UINT32 prepareMailDate(CHAR *pEmailBuf, UINT32 bufLen)
{
    CHAR        timezoneStr[10];
    struct tm   brokenTime = {0};

    /* Get timezone string and current local time */
    memset(timezoneStr, 0, sizeof(timezoneStr));
    GetTimezoneStr(timezoneStr);
    GetLocalTimeInBrokenTm(&brokenTime);

    /* RFC 822 Date Format: Thu, 23 May 2024 17:32:55 +0530 */
    return snprintf(pEmailBuf, bufLen, MAIL_ATTRIB_DATE "%s, %02d %s %04d %02d:%02d:%02d %s", GetWeekName(brokenTime.tm_wday), brokenTime.tm_mday,
                    GetMonthName(brokenTime.tm_mon), brokenTime.tm_year, brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec, timezoneStr);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare email body
 * @param   pEmailBuf
 * @param   bufLen
 * @param   pBody
 * @return  Number of characters of body added in email data
 */
static UINT32 prepareMailBody(CHAR *pEmailBuf, UINT32 bufLen, CHAR *pBody)
{
    UINT32 bodyLen = 0;

    /* Prapare Content-Type: Field */
    bodyLen += snprintf(&pEmailBuf[bodyLen], bufLen - bodyLen, MAIL_ATTRIB_CONTENT_TYPE "%s; ", PLAIN_CONTENT_TYPE);

    /* Add body without encoding if ascii characters string */
    if ((*pBody == '\0') || (TRUE == isAsciiString(pBody)))
    {
        /* Prapare charset: Field */
        bodyLen += snprintf(&pEmailBuf[bodyLen], bufLen - bodyLen, MAIL_ATTRIB_CHAR_SET "%s", CHAR_SET_USASCII);
        bodyLen += snprintf(&pEmailBuf[bodyLen], bufLen - bodyLen, CR_LF_STR);

        /* Prapare Content-Transfer-Encoding: Field */
        bodyLen += snprintf(&pEmailBuf[bodyLen], bufLen - bodyLen, MAIL_ATTRIB_CONTENT_ENCODING "%s", CHAR_ENCODING_7BIT);
        bodyLen += snprintf(&pEmailBuf[bodyLen], bufLen - bodyLen, CR_LF_STR CR_LF_STR);

        /* Write Mail Body */
        bodyLen += snprintf(&pEmailBuf[bodyLen], bufLen - bodyLen, "%s", pBody);
        bodyLen += snprintf(&pEmailBuf[bodyLen], bufLen - bodyLen, CR_LF_STR);
    }
    else
    {
        UINT32  dataCnt = 0;
        CHAR    *pBodyB64 = NULL;
        UINT32  bodyB64Len = 0;

        /* Prapare charset: Field */
        bodyLen += snprintf(&pEmailBuf[bodyLen], bufLen - bodyLen, MAIL_ATTRIB_CHAR_SET "%s", CHAR_SET_UTF8);
        bodyLen += snprintf(&pEmailBuf[bodyLen], bufLen - bodyLen, CR_LF_STR);

        /* Prapare Content-Transfer-Encoding: Field */
        bodyLen += snprintf(&pEmailBuf[bodyLen], bufLen - bodyLen, MAIL_ATTRIB_CONTENT_ENCODING "%s", CHAR_ENCODING_BASE64);
        bodyLen += snprintf(&pEmailBuf[bodyLen], bufLen - bodyLen, CR_LF_STR CR_LF_STR);

        /* Convert message body into base64 string */
        pBodyB64 = EncodeBase64(pBody, strlen(pBody));
        if (NULL == pBodyB64)
        {
            /* Failed to convert in base64 */
            return 0;
        }

        /* Get length of base64 string */
        bodyB64Len = strlen(pBodyB64);

        /* Only 80 characters are allowed in single line including "/r/n". Hence email body message
         * break down with appropriate single line length */
        while(bodyB64Len)
        {
            /* Breakdown email body data at 76 characters and add "\r\n" */
            if (bodyB64Len >= MAIL_ATTRIB_MAX_BYTES)
            {
                /* Add 76 characters in single line */
                snprintf(&pEmailBuf[bodyLen], MAIL_ATTRIB_MAX_BYTES + 1, "%s", &pBodyB64[dataCnt]);
                bodyLen += MAIL_ATTRIB_MAX_BYTES;
                dataCnt += MAIL_ATTRIB_MAX_BYTES;
                bodyB64Len -= MAIL_ATTRIB_MAX_BYTES;
            }
            else
            {
                /* Add last remainder characters in last line */
                snprintf(&pEmailBuf[bodyLen], bodyB64Len + 1, "%s", &pBodyB64[dataCnt]);
                bodyLen += bodyB64Len;
                dataCnt += bodyB64Len;
                bodyB64Len = 0;
            }

            /* Add "\r\n" in email message body line */
            bodyLen += snprintf(&pEmailBuf[bodyLen], bufLen - bodyLen, CR_LF_STR);
        }

        /* Free allocated base64 memory */
        FREE_MEMORY(pBodyB64);
    }

    /* Add an extra "\r\n" in email message body */
    bodyLen += snprintf(&pEmailBuf[bodyLen], bufLen - bodyLen, CR_LF_STR);

    /* Return number of added characters in email body */
    return bodyLen;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare email attachment
 * @param   pEmailBuf
 * @param   bufLen
 * @param   pAttachment
 * @param   pAttachInfo
 * @return  Number of characters of attachment added in email data
 */
static UINT32 prepareMailAttachment(CHAR *pEmailBuf, UINT32 bufLen, CHAR *pAttachment, EMAIL_ATTACHMENT_INFO_t *pAttachInfo)
{
    INT32               c1 = 0, c2 = 0, c3 = 0;
    UINT32              attachLen = 0, lineCharCnt = 0;
    static const CHAR   *pBase64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    /* Open email attachment file */
    FILE *pFile = fopen(pAttachment, "r");
    if (pFile == NULL)
    {
        /* Fail to open the file */
        return (0);
    }

    /* Prapare Content-Type: Field */
    attachLen += snprintf(&pEmailBuf[attachLen], bufLen - attachLen, MAIL_ATTRIB_CONTENT_TYPE "%s; ", pAttachInfo->contentType);
    attachLen += snprintf(&pEmailBuf[attachLen], bufLen - attachLen, MAIL_ATTRIB_NAME "\"%s\"", pAttachInfo->dispName);
    attachLen += snprintf(&pEmailBuf[attachLen], bufLen - attachLen, CR_LF_STR);

    /* Content-Disposition: attachment, filename=snapshot.jpg */
    attachLen += snprintf(&pEmailBuf[attachLen], bufLen - attachLen, MAIL_ATTRIB_CONTENT_DISPOSITION MAIL_ATTRIB_ATTACHMENT "; ");
    attachLen += snprintf(&pEmailBuf[attachLen], bufLen - attachLen, MAIL_ATTRIB_FILE_NAME "\"%s\"", pAttachInfo->dispName);
    attachLen += snprintf(&pEmailBuf[attachLen], bufLen - attachLen, CR_LF_STR);

    /* Content-Transfer-Encoding: base64 */
    attachLen += snprintf(&pEmailBuf[attachLen], bufLen - attachLen, MAIL_ATTRIB_CONTENT_ENCODING CHAR_ENCODING_BASE64);
    attachLen += snprintf(&pEmailBuf[attachLen], bufLen - attachLen, CR_LF_STR CR_LF_STR);

    /* Read character from file and convert into base64 */
    while ((c1 = getc(pFile)) != EOF)
    {
        /* Get next character from file */
        c2 = getc(pFile);

        /* Is it end of file? */
        if (c2 == EOF)
        {
            /* Only one characters found in bunch of 3 characters. Hence add last two characters as '=' */
            pEmailBuf[attachLen++] = pBase64[(c1 >> 2) & 0x3F];
            pEmailBuf[attachLen++] = pBase64[((c1 & 0x03) << 4) & 0x3F];
            pEmailBuf[attachLen++] = '=';
            pEmailBuf[attachLen++] = '=';
        }
        else
        {
            /* Get next character from file */
            c3 = getc(pFile);

            /* Is it end of file? */
            if (c3 == EOF)
            {
                /* Only two characters found in bunch of 3 characters. Hence add last one characters as '=' */
                pEmailBuf[attachLen++] = pBase64[(c1 >> 2) & 0x3F];
                pEmailBuf[attachLen++] = pBase64[(((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)) & 0x3F];
                pEmailBuf[attachLen++] = pBase64[((c2 & 0x0F) << 2) & 0x3F];
                pEmailBuf[attachLen++] = '=';
            }
            else
            {
                /* Convert all three characters into base64 */
                pEmailBuf[attachLen++] = pBase64[(c1 >> 2) & 0x3F];
                pEmailBuf[attachLen++] = pBase64[(((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)) & 0x3F];
                pEmailBuf[attachLen++] = pBase64[(((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6)) & 0x3F];
                pEmailBuf[attachLen++] = pBase64[c3 & 0x3F];
            }
        }

        /* We have converted 3 character into 4 base64 characters */
        lineCharCnt += 4;

        /* We cannot add more then 80 chracters in single line including "\r\n" */
        if (lineCharCnt < MAIL_ATTRIB_MAX_BYTES)
        {
            continue;
        }

        /* Add "\r\n" in every line of 76 characters */
        pEmailBuf[attachLen++] = '\r';
        pEmailBuf[attachLen++] = '\n';
        lineCharCnt = 0;
    }

    /* Close the file */
    fclose(pFile);

    /* Is last "\r\n" pending to add in line? */
    if (lineCharCnt)
    {
        pEmailBuf[attachLen++] = '\r';
        pEmailBuf[attachLen++] = '\n';
    }

    /* Add extra "\r\n\" in attachment */
    pEmailBuf[attachLen++] = '\r';
    pEmailBuf[attachLen++] = '\n';
    pEmailBuf[attachLen] = '\0';

    /* Return number of added characters in email body */
    return attachLen;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Send email using curl smtp request
 * @param   pSmtpConfig
 * @param   pQueueEntry
 * @param   pEmailPayloadInfo
 * @param   pRespErrCode
 * @return  Curl easy perform response code
 */
static CURLcode sendCurlSmtpRequest(const SMTP_CONFIG_t *pSmtpConfig, EMAIL_QUEUE_ENTRY_t *pQueueEntry,
                                    EMAIL_PAYLOAD_INFO_t *pEmailPayloadInfo, INT32 *pRespErrCode)
{
    CHAR                absoluteUrl[MAX_SMTP_SERVER_NAME_WIDTH] = "";
    CHAR                ipAddrForUrl[DOMAIN_NAME_SIZE_MAX] = "";
    CHAR                toField[MAX_EMAIL_ADDRESS_WIDTH];
    CURLcode            curlCode = CURLE_FAILED_INIT;
    long                response = 0;
    CHAR                *pStartPattern, *pEndPattern;
    CURL                *curl;
    struct curl_slist   *recipients = NULL;

    /* Convert ip address format for url */
    PrepareIpAddressForUrl(pSmtpConfig->server, ipAddrForUrl);

    /* Prepare smtp server url */
    snprintf(absoluteUrl, sizeof(absoluteUrl), "%s://%s:%d",
             (pSmtpConfig->encryptionType == SMTP_ENCRYPTION_TYPE_SSL) ? "smtps" : "smtp", ipAddrForUrl, pSmtpConfig->serverPort);

    /* Create curl session */
    curl = curl_easy_init();
    if (NULL == curl)
    {
        EPRINT(EMAIL_NOTIFY, "fail to execute curl_easy_init: [curlReturn=%d]", curlCode);
        return curlCode;
    }

    /* Set URL of mailserver */
    curl_easy_setopt(curl, CURLOPT_URL, absoluteUrl);

    /* Set from address (sender address) */
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, pSmtpConfig->senderAddress);

    /* Set username for server */
    curl_easy_setopt(curl, CURLOPT_USERNAME, pSmtpConfig->username);

    /* Set password for server */
    curl_easy_setopt(curl, CURLOPT_PASSWORD, pSmtpConfig->password);

    /* Add user-agent string */
    curl_easy_setopt(curl, CURLOPT_USERAGENT, curl_version());

    /* Enabel smtp and smtps */
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_SMTP | CURLPROTO_SMTPS);

    /* Disable verification of peer ssl certificate */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    /* Need to enable tls? */
    if (pSmtpConfig->encryptionType == SMTP_ENCRYPTION_TYPE_TLS)
    {
        /* Enable SSL/TLS - - SSL for all communication or fail */
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    }

    /* Add recipients in the recipient list */
    snprintf(toField, sizeof(toField), "%s", pQueueEntry->toField);
    pStartPattern = toField;
    while(TRUE)
    {
        /* Are multiple email receivers added? */
        pEndPattern = strchr(pStartPattern, ',');
        if (pEndPattern == NULL)
        {
            /* No email available next */
            recipients = curl_slist_append(recipients, pStartPattern);
            break;
        }

        /* Other emails are available in list */
        *pEndPattern = '\0';

        /* Add current email in receiver list */
        recipients = curl_slist_append(recipients, pStartPattern);

        /* Now get next receiver email address */
        pStartPattern = pEndPattern + 1;
    }

    /* Add all receivers email in recipient list */
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    /* Allow RCPT TO command to fail for some recipients (This enum value is not available in old curl) */
    #if !defined(HI3536_NVRL) && !defined(HI3536_NVRH)
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT_ALLLOWFAILS, 1L);
    #endif

    /* TCP connection timeout */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

    /* Complete operation within given seconds */
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    /* Option to inform curl to do not use signal */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    /* Low speed limit & time */
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 100L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 4L);

    /* Inform curl to stop all progress meter */
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

    /* We are using a callback function to specify the payload (the headers and body of the message).
     * We will pass paylod information including attachment in CURLOPT_READDATA option to upload data */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, emailPayalodUploadCb);
    curl_easy_setopt(curl, CURLOPT_READDATA, pEmailPayloadInfo);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    /* Perform smtp request using curl easy perform */
    curlCode = curl_easy_perform(curl);
    switch(curlCode)
    {
        case CURLE_OK:
        case CURLE_SEND_ERROR:
        case CURLE_RECV_ERROR:
        {
            /* Last arg must be long ptr or ptr to char* */
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);

            /* SMTP response code */
            *pRespErrCode = (INT32)response;
        }
        break;

        default:
        {
            /* Last arg must be long ptr or ptr to char* */
            curl_easy_getinfo(curl, CURLINFO_OS_ERRNO, &response);

            /* OS error code */
            *pRespErrCode = (INT32)response;
        }
        break;
    }

    /* Free recipients list */
    if (NULL != recipients)
    {
        curl_slist_free_all(recipients);
    }

    /* Free curl handle */
    curl_easy_cleanup(curl);

    /* Return curl response code */
    return curlCode;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Email data payload upload callback
 * @param   pUploadData
 * @param   size
 * @param   nmemb
 * @param   pUserData
 * @return
 */
static size_t emailPayalodUploadCb(CHAR *pUploadData, size_t size, size_t nmemb, void *pUserData)
{
    size_t                  uploadLen;
    size_t                  requestLen = size * nmemb;
    EMAIL_PAYLOAD_INFO_t    *pEmailPayloadInfo = (EMAIL_PAYLOAD_INFO_t *)pUserData;

    /* If no data requested then return zero */
    if ((size == 0) || (nmemb == 0) || (requestLen < 1))
    {
        return 0;
    }

    /* If no data in payload then return zero */
    if (pEmailPayloadInfo->totalLength == 0)
    {
        return 0;
    }

    /* Get required upload length and copy data in upload buffer */
    uploadLen = (requestLen < pEmailPayloadInfo->totalLength) ? requestLen : pEmailPayloadInfo->totalLength;
    memcpy(pUploadData, &pEmailPayloadInfo->pEmailData[pEmailPayloadInfo->bytesRead], uploadLen);
    pEmailPayloadInfo->bytesRead += uploadLen;
    pEmailPayloadInfo->totalLength -= uploadLen;

    /* Return uploaded data length */
    return uploadLen;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write failure event log in case of error
 * @param   pQueueEntry
 * @param   curlCode
 * @param   respErrCode
 * @param   pEmailRetryCnt
 * @return  Returns TRUE of retry not needed else returns FALSE
 */
static BOOL writeEmailErrInEventLog(EMAIL_QUEUE_ENTRY_t *pQueueEntry, CURLcode curlCode, INT32 respErrCode, UINT8 *pEmailRetryCnt)
{
    /* Is email sent successfully? */
    if (curlCode == CURLE_OK)
    {
        (*pEmailRetryCnt) = 0;
        DPRINT(EMAIL_NOTIFY, "email sent successfully: [toField=%s]", pQueueEntry->toField);
        return TRUE;
    }

    /* Is failure from smtp server? */
    if ((curlCode == CURLE_SEND_ERROR) || (curlCode == CURLE_RECV_ERROR))
    {
        /* If mail sending failed because of connection error, we increment retry count to try again later.
         * If we already reached max retry, we will discard it and will move to next mail. */
        if ((respErrCode == SMTP_SERVER_UNAVAILABLE) || (respErrCode == SMTP_RECIPIENT_SERVER_NOT_RESPONDING)
                || (respErrCode == SMTP_SERVER_STORAGE_FULL) || (respErrCode == SMTP_MAIL_NOT_ACCEPTED))
        {
            /* Wait for some time and try again */
            if(*pEmailRetryCnt < EMAIL_RETRY_CNT_MAX)
            {
                /* Update email retry counter */
                (*pEmailRetryCnt)++;
                EPRINT(EMAIL_NOTIFY, "fail to send mail, retry after timeout: [toField=%s], [curlCode=%d], [smtpErrCode=%d]",
                       pQueueEntry->toField, curlCode, respErrCode);
                return FALSE;
            }

            /* Reset retry count and discard the email entry */
            (*pEmailRetryCnt) = 0;
        }

        /* Log event as per smtp response code */
        switch(respErrCode)
        {
            case SMTP_SERVER_UNAVAILABLE:
                WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "SMTP Server not available at the moment", EVENT_FAIL);
                break;

            case SMTP_RECIPIENT_SERVER_NOT_RESPONDING:
                WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Recipient SMTP server not responding", EVENT_FAIL);
                break;

            case SMTP_SERVER_STORAGE_FULL:
                WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Server storage limit exceeded", EVENT_FAIL);
                break;

            case SMTP_MAIL_NOT_ACCEPTED:
                WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Mail is not accepted", EVENT_FAIL);
                break;

            case SMTP_CONNECTION_ERROR:
                WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Could not connect to SMTP Server or Port", EVENT_FAIL);
                break;

            case SMTP_RECIPIENT_SERVER_FULL:
            case SMTP_RECIPIENT_MAILBOX_FULL:
                WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Recipient mailbox full", EVENT_FAIL);
                break;

            case SMTP_CONNNECTION_REFUSED:
                WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Connection refused by SMTP server", EVENT_FAIL);
                break;

            case SMTP_RECIPIENT_MAILBOX_UNAVAILABLE:
            case SMTP_INVALID_SYNTEX:
            case SMTP_BAD_SEQUENCE:
            case SMTP_INVALID_PARAMETER:
            case SMTP_BAD_EMAIL_ADDR_CODE1:
            case SMTP_BAD_EMAIL_ADDR_CODE2:
            case SMTP_DNS_ERROR:
            case SMTP_INCORRECT_EMAIL_ADDR:
            case SMTP_EMAIL_NOT_EXIST:
            case SMTP_INVALID_MAILBOX_ADDR:
                WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Sender/Recipient Mailbox address invalid or unavailable", EVENT_FAIL);
                break;

            case SMTP_INVALID_AUTHENTICATION:
            case SMTP_APP_AUTHENTICATION:
            case SMTP_AUTHENTICATION_FAILED:
                WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Authentication Failed. Verify Entered Details.", EVENT_FAIL);
                break;

            case SMTP_TRANSACTION_FAILED:
                WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Transaction failed. Email is Spam/Blacklisted", EVENT_FAIL);
                break;

            default:
                WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Email Notification Failed.", EVENT_FAIL);
                break;
        }

        /* It is failure from SMTP server. Email retry is not needed */
        EPRINT(EMAIL_NOTIFY, "fail to send mail, smtp server error, email discarded: [toField=%s], [curlCode=%d], [smtpErrCode=%d]",
               pQueueEntry->toField, curlCode, respErrCode);
        return TRUE;
    }

    /* Handle local failure cases */
    switch(curlCode)
    {
        case CURLE_LOGIN_DENIED:
            WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Authentication Failed. Verify Entered Details.", EVENT_FAIL);
            break;

        case CURLE_OPERATION_TIMEDOUT:
            WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Operation timed-out. Please try later", EVENT_FAIL);
        break;

        case CURLE_GOT_NOTHING:
            WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "No response received", EVENT_FAIL);
            break;

        case CURLE_OUT_OF_MEMORY:
            WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Internal memory issue", EVENT_FAIL);
            break;

        case CURLE_SSL_CONNECT_ERROR:
            WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "SSL connection error", EVENT_FAIL);
            break;

        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_CONNECT:
        default:
            WriteEvent(LOG_NETWORK_EVENT, LOG_EMAIL_NOTIFICATION, NULL, "Error in connection. Please try later", EVENT_FAIL);
            break;
    }

    /* It is local failure. Email retry is not needed */
    EPRINT(EMAIL_NOTIFY, "fail to send mail, local failure, email discarded: [toField=%s], [curlCode=%d], [errno=%d]",
           pQueueEntry->toField, curlCode, respErrCode);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get perform email request response code for quick email
 * @param   emailType
 * @param   curlCode
 * @param   respErrCode
 * @return  Command response code
 */
static NET_CMD_STATUS_e getQuickEmailCmdRespCode(QUICK_EMAIL_TYPE_e emailType, CURLcode curlCode, INT32 respErrCode)
{
    /* Is curl response ok? */
    if (curlCode == CURLE_OK)
    {
        DPRINT(EMAIL_NOTIFY, "%s mail sent successfully", getQuickEmailTypeStr(emailType));
        return CMD_SUCCESS;
    }

    /* Is failure from smtp server? */
    if ((curlCode == CURLE_SEND_ERROR) || (curlCode == CURLE_RECV_ERROR))
    {
        EPRINT(EMAIL_NOTIFY, "fail to send %s, smtp server error: [curlCode=%d], [smtpErrCode=%d]", getQuickEmailTypeStr(emailType), curlCode, respErrCode);

        switch(respErrCode)
        {
            case SMTP_CONNECTION_ERROR:
                return CMD_SMTP_SERVER_CONNECTION_ERROR;

            case SMTP_SERVER_UNAVAILABLE:
                return CMD_SMTP_SERVER_UNAVAILABLE;

            case SMTP_RECIPIENT_SERVER_FULL:
            case SMTP_RECIPIENT_MAILBOX_FULL:
                return CMD_SMTP_RECIPIENT_MAILBOX_FULL;

            case SMTP_RECIPIENT_SERVER_NOT_RESPONDING:
                return CMD_SMTP_RECIPIENT_SERVER_NOT_RESPONDING;

            case SMTP_MAIL_NOT_ACCEPTED:
                return CMD_SMTP_MAIL_NOT_ACCEPTED;

            case SMTP_CONNNECTION_REFUSED:
                return CMD_SMTP_CONNECTION_REFUSED;

            case SMTP_AUTHENTICATION_FAILED:
            case SMTP_APP_AUTHENTICATION:
            case SMTP_INVALID_AUTHENTICATION:
                return CMD_SMTP_AUTHENTICATION_FAILED;

            case SMTP_RECIPIENT_MAILBOX_UNAVAILABLE:
            case SMTP_INVALID_SYNTEX:
            case SMTP_BAD_SEQUENCE:
            case SMTP_INVALID_PARAMETER:
            case SMTP_BAD_EMAIL_ADDR_CODE1:
            case SMTP_BAD_EMAIL_ADDR_CODE2:
            case SMTP_DNS_ERROR:
            case SMTP_INCORRECT_EMAIL_ADDR:
            case SMTP_EMAIL_NOT_EXIST:
            case SMTP_INVALID_MAILBOX_ADDR:
                return CMD_SMTP_INVALID_EMAIL_ADDR;

            case SMTP_SERVER_STORAGE_FULL:
                return CMD_SMTP_SERVER_STORAGE_FULL;

            case SMTP_TRANSACTION_FAILED:
                return CMD_SMTP_TRANSACTION_FAILED;

            default:
                return CMD_TESTING_FAIL;
        }
    }

    /* It is local failure */
    EPRINT(EMAIL_NOTIFY, "fail to send %s, local failure: [curlCode=%d], [errno=%d]", getQuickEmailTypeStr(emailType), curlCode, respErrCode);
    switch(curlCode)
    {
        case CURLE_LOGIN_DENIED:
            return CMD_SMTP_AUTHENTICATION_FAILED;

        default:
            return CMD_TESTING_FAIL;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get quick email type string
 * @param   emailType
 * @return  Email type string
 */
static const CHAR *getQuickEmailTypeStr(QUICK_EMAIL_TYPE_e emailType)
{
    switch(emailType)
    {
        case QUICK_EMAIL_TYPE_TEST:
            return ("TEST_EMAIL");

        case QUICK_EMAIL_TYPE_OTP:
            return ("OTP_EMAIL");

        default:
            return ("UNKNOWN_EMAIL");
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
