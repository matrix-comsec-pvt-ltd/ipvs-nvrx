//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file       FtpClient.c
@brief      This module provides functionality to perform upload and download to/ from FTP server.
            When a transfer request is invoked, a new entry is made in request list table with all
            corresponding information. It will allow to concurrent request till maximum request of
            FTP client not reached. Once request is accepted, a thread to process the request is
            created and actual data transfer is done in this thread. After the transfer is completed
            or if an error occurs it invokes user registered callback function returning transfer
            result to module initiating the transfer. Thereafter, entry is removed from the request list.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Curl Library Includes */
#include <curl/curl.h>

/* Application Includes */
#include "Utils.h"
#include "FtpClient.h"
#include "DebugLog.h"
#include "DateTime.h"
#include "NetworkController.h"
#include "Queue.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
// Total number of file was queue for each channel
#define MAX_FTP_REQUEST			8
#define MAX_FTP_IMG_UPLOAD_SIZE	(getMaxCameraForCurrentVariant() * 2)

// ftp connection timeout
#define FTP_CONNECTION_TIMEOUT	30L
#define FTP_FILE_SEPERATOR 		'/'
#define FTP_TEST_STRING			"This is testing message sent by the device"
#define FTP_CONN_REFRESH_TIME	(5)

#define MAX_FTP_CONN_RETRY      4

/* Use Default Stack Size*/
#define FTP_THREAD_STACK_SZ     (2*MEGA_BYTE)
#define IMG_UPLOAD_STACK_SZ     (2*MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
// structure which contains all parameters, required during communication
typedef enum 
{
	FTP_UPLOAD,
	FTP_DOWNLOAD,
	FTP_MAX_REQUEST_TYPE

}FTP_REQUEST_TYPE_e;

// structure which contains all parameters, required during communication with FTP server for uploading and downloading
typedef struct
{
	BOOL					requestStatus;	// Indicates request in which state
	FTP_REQUEST_TYPE_e		requestType;	// UPLOAD or DOWNLOAD

	// actual FTP handle provided by LibCurl
    CURL					*curlHandle;	// actual FTP handle provided by LibCurl
	FTP_FILE_INFO_t 		ftpInfo;		// points to FTP parameter which is to be upload

	// points to user callback function
	FTP_CALLBACK			userCallback;
	UINT16  				userData;		// give back to user

	// thread terminate flag, used when user want to cancel the ftp transfer implicitly
	BOOL					terminateFlg;
	FTP_HANDLE				ftpSessionHandle;

	//authentication for FTP
	CHAR 					userName[MAX_FTP_USERNAME_WIDTH];
	CHAR 					password[MAX_FTP_PASSWORD_WIDTH];

}FTP_REQUEST_LIST_t;

typedef struct
{
	BOOL 					runStatus;
	NW_CMD_REPLY_CB   		callback;
	INT32			  		connFd;
	pthread_mutex_t			runStatusMutex;
	CHAR			  		localFileName[FTP_FILE_NAME_SIZE];

}FTP_CONN_TEST_t;

typedef struct
{
	VOIDPTR 				userData;
	CHARPTR					dataPtr;
	UINT32					size;
	FTP_RESPONSE_e			retVal;
	BOOL 					(*callback)(CHARPTR list, UINT32 size, VOIDPTR userData);

}FTP_LIST_INFO_t;

typedef struct
{
	FTP_UPLOAD_CONFIG_t		*ftpCfg;
	CHARPTR					path;
	FTP_RESPONSE_e			retVal;
	FTP_SYS_e				sysType;

}FTP_DEL_INFO_t;

typedef struct
{
	UINT8					ftpIdx;
	QUEUE_HANDLE			ftpQHandle;

}FTP_IMG_UPLD_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static FTP_REQUEST_LIST_t   ftpRequestList[MAX_FTP_REQUEST];
static pthread_mutex_t      ftpDataMutex;
static FTP_CONN_TEST_t      ftpConnTest;
static FTP_IMG_UPLD_t       ftpImgUpld[MAX_FTP_SERVER];

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static FTP_HANDLE startFtpTransfer(FTP_UPLOAD_CONFIG_t *ftpConfigPtr, FTP_FILE_INFO_t *fileInfo, BOOL isMatrixRecDir,
                                   FTP_REQUEST_TYPE_e requestType, FTP_CALLBACK ftpCallback, UINT16 userData, BOOL imageUpload);
//-------------------------------------------------------------------------------------------------
static VOIDPTR imageUploadThread(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR ftpThread(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static FTP_RESPONSE_e ftpTransfer(FTP_REQUEST_LIST_t *sessionPtr);
//-------------------------------------------------------------------------------------------------
static INT32 abortFtpTransfer(VOIDPTR clientp, double dltotal, double dlnow, double ultotal, double ulnow);
//-------------------------------------------------------------------------------------------------
static void testFtpConnCallback(FTP_HANDLE ftpHandle, FTP_RESPONSE_e ftpResponse, UINT16 userData);
//-------------------------------------------------------------------------------------------------
static size_t curlListCb(VOIDPTR buffer, size_t row, size_t column, VOIDPTR stream);
//-------------------------------------------------------------------------------------------------
static BOOL delFileCb(CHARPTR list, UINT32 size, VOIDPTR userData);
//-------------------------------------------------------------------------------------------------
static size_t curlVerboseCb(VOIDPTR buffer, size_t row, size_t column, VOIDPTR stream);
//-------------------------------------------------------------------------------------------------
static FTP_RESPONSE_e getFtpRespFromCurlResp(CURLcode performStatus);
//-------------------------------------------------------------------------------------------------
static void queueFullCb(VOIDPTR entry);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes FTP client module. And also initialize all  request to default state
 * @return  SUCCESS/FAIL
 */
BOOL InitFtpClient(void)
{
	UINT8			loop;
	QUEUE_INIT_t	qInfo;

	// Initialize the ftpDataMutex
    MUTEX_INIT(ftpDataMutex, NULL);

	for(loop = 0; loop < MAX_FTP_REQUEST; loop++)
	{
		ftpRequestList[loop].requestStatus = FREE;
		ftpRequestList[loop].ftpSessionHandle = loop;
		ftpRequestList[loop].curlHandle = NULL;
	}

	QUEUE_PARAM_INIT(qInfo);
	qInfo.maxNoOfMembers = MAX_FTP_IMG_UPLOAD_SIZE;
	qInfo.sizoOfMember = sizeof(FTP_REQUEST_LIST_t);
	qInfo.syncRW = TRUE;
	qInfo.overwriteOldest = TRUE;
	qInfo.callback = queueFullCb;

	for(loop = 0; loop < MAX_FTP_SERVER; loop++)
	{
		ftpImgUpld[loop].ftpIdx = loop;
		ftpImgUpld[loop].ftpQHandle = QueueCreate(&qInfo);

        if (FAIL == Utils_CreateThread(NULL, imageUploadThread, &ftpImgUpld[loop], DETACHED_THREAD, IMG_UPLOAD_STACK_SZ))
        {
            DPRINT(FTP_CLIENT, "fail to create ftp client thread: [ftp=%d]", loop);
        }
	}

    MUTEX_INIT(ftpConnTest.runStatusMutex, NULL);
	ftpConnTest.runStatus = FREE;
	ftpConnTest.callback = NULL;
	ftpConnTest.connFd = INVALID_CONNECTION;
	return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function de initialize FTP client module. And also initialize all request to its default state.
 * @return  SUCCESS/FAIL
 */
BOOL DeinitFtpClient(void)
{
	UINT8	ftpRequestCnt;
	
	for(ftpRequestCnt = 0; ftpRequestCnt < MAX_FTP_REQUEST; ftpRequestCnt++)
	{
		StopFtpTransfer(ftpRequestCnt);
	}

    DPRINT(FTP_CLIENT, "ftp client de-init successfully");
	return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initiates the uploading data to given ftp server. It will create a thread
 *          for this request and upload data is performed in this thread. Result of upload request
 *          is passed back to caller with callback.
 * @param   fileInfo
 * @param   ftpCallback
 * @param   userData
 * @param   ftpHandle
 * @return  NET_CMD_STATUS_e
 */
NET_CMD_STATUS_e StartFtpUpload(FTP_FILE_INFO_t *fileInfo, FTP_CALLBACK ftpCallback, UINT16 userData, FTP_HANDLE * ftpHandle)
{
    FTP_UPLOAD_CONFIG_t ftpUpldCfg;

    if ((ftpHandle == NULL) || (fileInfo == NULL))
	{
        EPRINT(FTP_CLIENT, "invld input params");
        return CMD_PROCESS_ERROR;
    }

    *ftpHandle = INVALID_FTP_HANDLE;
    if(fileInfo->ftpServer >= MAX_FTP_SERVER)
    {
        EPRINT(FTP_CLIENT, "invld ftp server: [ftpServer=%d]", fileInfo->ftpServer);
        return CMD_PROCESS_ERROR;
    }

    ReadSingleFtpUploadConfig(fileInfo->ftpServer, &ftpUpldCfg);
    if(ftpUpldCfg.ftp == DISABLE)
    {
        EPRINT(FTP_CLIENT, "ftp server disable: [ftpServer=%d]", fileInfo->ftpServer);
        return CMD_SERVER_DISABLED;
    }

    *ftpHandle = startFtpTransfer(&ftpUpldCfg, fileInfo, TRUE, FTP_UPLOAD, ftpCallback, userData, FALSE);
    if(*ftpHandle >= MAX_FTP_REQUEST)
    {
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initiates the uploading data to given ftp server. It will create a thread
 *          for this request and upload data is performed in this thread. Result of upload request
 *          is passed back to caller with callback.
 * @param   camIndex
 * @param   fileInfo
 * @return  NET_CMD_STATUS_e
 */
NET_CMD_STATUS_e FtpImageUpload(UINT8 camIndex, FTP_FILE_INFO_t * fileInfo)
{
	FTP_UPLOAD_CONFIG_t	ftpUpldCfg;

    if ((fileInfo == NULL) || (fileInfo->ftpServer >= MAX_FTP_SERVER))
    {
        EPRINT(FTP_CLIENT, "invld input: [camera=%d]", camIndex);
        return CMD_PROCESS_ERROR;
    }

    ReadSingleFtpUploadConfig(fileInfo->ftpServer, &ftpUpldCfg);
    if (ftpUpldCfg.ftp == DISABLE)
    {
        EPRINT(FTP_CLIENT, "ftp server disable: [camera=%d], [ftpServer=%d]", camIndex, fileInfo->ftpServer);
        return CMD_SERVER_DISABLED;
    }

    startFtpTransfer(&ftpUpldCfg, fileInfo, TRUE, FTP_UPLOAD, NULL, camIndex, TRUE);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Start firmware download from FTP to local dir
 * @param   ftpServerCfg
 * @param   fileInfo
 * @param   ftpCallback
 * @param   userData
 * @return
 */
NET_CMD_STATUS_e StartFirmwareDownload(FTP_UPLOAD_CONFIG_t *ftpServerCfg, FTP_FILE_INFO_t *fileInfo, FTP_CALLBACK ftpCallback, UINT16 userData)
{
    if ((ftpServerCfg == NULL) || (fileInfo == NULL))
    {
        EPRINT(FTP_CLIENT, "invld input params");
        return CMD_PROCESS_ERROR;
    }

    if (startFtpTransfer(ftpServerCfg, fileInfo, FALSE, FTP_DOWNLOAD, ftpCallback, userData, FALSE) >= MAX_FTP_REQUEST)
    {
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   startFtpTransfer
 * @param   ftpConfigPtr
 * @param   fileInfo
 * @param   ftpCallback
 * @param   userData
 * @param   imageUpload
 * @return  FTP handle
 */
static FTP_HANDLE startFtpTransfer(FTP_UPLOAD_CONFIG_t *ftpConfigPtr, FTP_FILE_INFO_t *fileInfo, BOOL isMatrixRecDir,
                                   FTP_REQUEST_TYPE_e requestType, FTP_CALLBACK ftpCallback, UINT16 userData, BOOL imageUpload)
{
    FTP_HANDLE          ftpHandle = INVALID_FTP_HANDLE;
	CHAR  				uploadLoc[FTP_REMOTE_PATH_LEN];
    CHAR                macAddress[MAX_MAC_ADDRESS_WIDTH + 1]; // MAC address + Separator
	FTP_REQUEST_LIST_t	*sessionPtr = NULL;
	FTP_REQUEST_LIST_t	imageReq;
    CHAR                ipAddressForUrl[DOMAIN_NAME_SIZE_MAX] = { 0 };

	if (imageUpload == TRUE)
	{
		sessionPtr = &imageReq;
	}
	else
	{
        // find out free available session
        MUTEX_LOCK(ftpDataMutex);
        for(ftpHandle = 0; ftpHandle < MAX_FTP_REQUEST; ftpHandle++)
		{
            if(ftpRequestList[ftpHandle].requestStatus == FREE)
			{
                ftpRequestList[ftpHandle].requestStatus = BUSY;
				break;
			}
		}
        MUTEX_UNLOCK(ftpDataMutex);

		// request was reached maximum ftp request
        if (ftpHandle >= MAX_FTP_REQUEST)
		{
            return INVALID_FTP_HANDLE;
        }

        sessionPtr = &ftpRequestList[ftpHandle];
	}

    /* We have to append mac address in matrix recording directory path */
    memset(macAddress, '\0', sizeof(macAddress));
    if (isMatrixRecDir == TRUE)
    {
        GetMacAddrPrepare(LAN1_PORT, macAddress);
        snprintf(&macAddress[strlen(macAddress)], sizeof(macAddress) - strlen(macAddress), "%c", FTP_FILE_SEPERATOR);
    }

    if(ftpConfigPtr->uploadPath[0] != '\0')
    {
        snprintf(uploadLoc, FTP_REMOTE_PATH_LEN, "%c%s%c%s",
                 FTP_FILE_SEPERATOR, ftpConfigPtr->uploadPath, FTP_FILE_SEPERATOR, macAddress);
    }
    else
    {
        snprintf(uploadLoc, FTP_REMOTE_PATH_LEN, "%c%s", FTP_FILE_SEPERATOR, macAddress);
    }

    /* Prepare FTP transfer information including absolute Url to transfer ftp file */
    sessionPtr->requestType = requestType;

    PrepareIpAddressForUrl(ftpConfigPtr->server, ipAddressForUrl);

    snprintf(sessionPtr->ftpInfo.remoteFile, FTP_REMOTE_PATH_LEN, "ftp://%s:%d%s%c%s",
             ipAddressForUrl, ftpConfigPtr->serverPort, uploadLoc, FTP_FILE_SEPERATOR, fileInfo->remoteFile);

    snprintf(sessionPtr->ftpInfo.localFileName, FTP_FILE_NAME_SIZE, "%s", fileInfo->localFileName);
    snprintf(sessionPtr->userName, MAX_FTP_USERNAME_WIDTH, "%s", ftpConfigPtr->username);
    snprintf(sessionPtr->password, MAX_FTP_PASSWORD_WIDTH, "%s", ftpConfigPtr->password);

    sessionPtr->userCallback = ftpCallback;
    sessionPtr->userData = userData;
    sessionPtr->curlHandle = NULL;
    sessionPtr->terminateFlg = FALSE;

    if (imageUpload == FALSE)
    {
        if (FAIL == Utils_CreateThread(NULL, ftpThread, sessionPtr, DETACHED_THREAD, FTP_THREAD_STACK_SZ))
        {
            EPRINT(FTP_CLIENT, "fail to create ftp session thread: [ftpHandle=%d]", ftpHandle);
            MUTEX_LOCK(ftpDataMutex);
            sessionPtr->requestStatus = FREE;
            MUTEX_UNLOCK(ftpDataMutex);
            return INVALID_FTP_HANDLE;
        }
    }
    else
    {
        QueueAddEntry(ftpImgUpld[fileInfo->ftpServer].ftpQHandle, sessionPtr);
    }

    return ftpHandle;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function implicitly cancels the ftp transfer for given session and free this request into request table.
 * @param   ftpHandle
 * @return  SUCCESS/FAIL
 */
BOOL StopFtpTransfer(FTP_HANDLE ftpHandle)
{
    if (ftpHandle >= MAX_FTP_REQUEST)
	{
        return FAIL;
    }

    // check that request for this handle was running
    MUTEX_LOCK(ftpDataMutex);
    if(ftpRequestList[ftpHandle].requestStatus == BUSY)
    {
        ftpRequestList[ftpHandle].terminateFlg = TRUE;
    }
    MUTEX_UNLOCK(ftpDataMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was request to FTP with fixed text file for checking connectivity with FTP server
 * @param   ftpDataPtr
 * @param   callback
 * @param   connFd
 * @return  NET_CMD_STATUS_e
 */
NET_CMD_STATUS_e TestFtpConn(FTP_UPLOAD_CONFIG_t * ftpDataPtr, NW_CMD_REPLY_CB callback, INT32 connFd)
{
	NET_CMD_STATUS_e 	status = CMD_PROCESS_ERROR;
	INT32 				fileFd, writeCnt;
	struct tm 			brokenTime = { 0 };
	FTP_FILE_INFO_t 	ftpFileInfo;

	do
	{
        MUTEX_LOCK(ftpConnTest.runStatusMutex);
		if(ftpConnTest.runStatus == BUSY)
		{
            MUTEX_UNLOCK(ftpConnTest.runStatusMutex);
			status = CMD_TESTING_ON;
			break;
		}
		ftpConnTest.runStatus = BUSY;
        MUTEX_UNLOCK(ftpConnTest.runStatusMutex);

		if(SUCCESS != GetLocalTimeInBrokenTm(&brokenTime))
		{
            EPRINT(FTP_CLIENT, "fail to get local time in broken");
		}

        snprintf(ftpFileInfo.remoteFile, FTP_REMOTE_PATH_LEN, "test-%02d%02d%04d_%02d%02d%02d.txt",
                 brokenTime.tm_mday, brokenTime.tm_mon+1, brokenTime.tm_year, brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec);
        snprintf(ftpFileInfo.localFileName, FTP_FILE_NAME_SIZE, "%s%s", "/tmp/", ftpFileInfo.remoteFile);

        fileFd = open(ftpFileInfo.localFileName, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
		if(fileFd == INVALID_FILE_FD)
		{
            EPRINT(FTP_CLIENT, "fail to open file: [path=%s], [err=%s]", ftpFileInfo.localFileName, STR_ERR);
			status = CMD_TESTING_FAIL;
			break;
		}

		writeCnt = write(fileFd, FTP_TEST_STRING, strlen(FTP_TEST_STRING));
		close(fileFd);

		if((size_t)writeCnt != strlen(FTP_TEST_STRING))
		{
            EPRINT(FTP_CLIENT, "fail to write file: [path=%s], [err=%s]", ftpFileInfo.localFileName, STR_ERR);
			remove(ftpFileInfo.localFileName);
			status = CMD_TESTING_FAIL;
			break;
		}

		ftpFileInfo.ftpServer = MAX_FTP_SERVER;
        if(startFtpTransfer(ftpDataPtr, &ftpFileInfo, TRUE, FTP_UPLOAD, testFtpConnCallback, 0, FALSE) >= MAX_FTP_REQUEST)
		{
			status = CMD_TESTING_FAIL;
			unlink(ftpFileInfo.localFileName);
			break;
		}

		ftpConnTest.connFd = connFd;
		ftpConnTest.callback = callback;
        snprintf(ftpConnTest.localFileName, FTP_FILE_NAME_SIZE, "%s", ftpFileInfo.localFileName);
		status = CMD_SUCCESS;
	}
	while(0);

    if((status != CMD_SUCCESS) && (status != CMD_TESTING_ON))
	{
        MUTEX_LOCK(ftpConnTest.runStatusMutex);
		ftpConnTest.runStatus = FREE;
        MUTEX_UNLOCK(ftpConnTest.runStatusMutex);
        ftpConnTest.connFd = INVALID_CONNECTION;
		ftpConnTest.callback = NULL;
	}

	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives list of files in given directory
 * @param   ftpCfg
 * @param   sysType
 * @param   isMatrixRecDir
 * @param   path
 * @param   userData
 * @return  FTP_RESPONSE_e
 */
FTP_RESPONSE_e ListFromFtp(FTP_UPLOAD_CONFIG_t *ftpCfg, FTP_SYS_e sysType, BOOL isMatrixRecDir, CHARPTR path, VOIDPTR userData,
                           BOOL (*callback)(CHARPTR list, UINT32 size, VOIDPTR userData))
{
    FTP_RESPONSE_e          retVal = FTP_FAIL;
    CURL                    *curl;
    UINT16                  outLen = 0;
    CURLcode                performStatus;
    CHAR                    macAddress[MAX_MAC_ADDRESS_WIDTH + 1]; // MAC address + Separator
    CHAR                    remoteFile[FTP_REMOTE_PATH_LEN];
    struct curl_slist       *list = NULL;
    static const CHARPTR    changeDirStyle = "SITE DIRSTYLE";
    FTP_LIST_INFO_t         listInfo = {.userData = userData, .dataPtr = NULL, .callback = callback, .retVal = FTP_SUCCESS, .size = 0};
    CHAR                    ipAddressForUrl[DOMAIN_NAME_SIZE_MAX] = { 0 };

    if ((callback == NULL) || (ftpCfg == NULL))
	{
        return FTP_FAIL;
    }

    /* We have to append mac address in matrix recording directory path */
    memset(macAddress, '\0', sizeof(macAddress));
    if (isMatrixRecDir == TRUE)
    {
        GetMacAddrPrepare(LAN1_PORT, macAddress);
        snprintf(&macAddress[strlen(macAddress)], sizeof(macAddress) - strlen(macAddress), "%c", FTP_FILE_SEPERATOR);
    }

    PrepareIpAddressForUrl(ftpCfg->server, ipAddressForUrl);

    outLen += snprintf(remoteFile, FTP_REMOTE_PATH_LEN, "ftp://%s:%d/", ipAddressForUrl, ftpCfg->serverPort);

    if (ftpCfg->uploadPath[0] != '\0')
    {
        outLen += snprintf(remoteFile + outLen, FTP_REMOTE_PATH_LEN, "%s/", ftpCfg->uploadPath);
    }

    outLen += snprintf(remoteFile + outLen, FTP_REMOTE_PATH_LEN, "%s", macAddress);

    if (path != NULL)
    {
        outLen += snprintf(remoteFile + outLen, FTP_REMOTE_PATH_LEN, "%s", path);
    }

    if (outLen >= FTP_REMOTE_PATH_LEN)
    {
        EPRINT(FTP_CLIENT, "buffer is too small for remote path: [buffer=%d], [outLen=%d]", FTP_REMOTE_PATH_LEN, outLen);
        return FTP_FAIL;
    }

    DPRINT(FTP_CLIENT, "list for dir: [path=%s]", remoteFile);

    /* Create curl session */
    curl = curl_easy_init();
    if (curl == NULL)
    {
        EPRINT(FTP_CLIENT, "curl init fail");
        return FTP_FAIL;
    }

    /* Check FTP system type */
    if (sysType == FTP_SYS_WINDOWS_NT)
    {
        /* Add directory style in header for windows */
        list = curl_slist_append(list, changeDirStyle);
        curl_easy_setopt(curl, CURLOPT_QUOTE, list);
    }

    /* Add option to inform curl to do not use signal */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    /* Add curl URL in header */
    curl_easy_setopt(curl, CURLOPT_URL, remoteFile);

    /* Add username of server */
    curl_easy_setopt(curl, CURLOPT_USERNAME, ftpCfg->username);

    /* Add password of server */
    curl_easy_setopt(curl, CURLOPT_PASSWORD, ftpCfg->password);

    /* Set callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlListCb);

    /* Pass fetch struct pointer */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &listInfo);

    /* TCP connection timeout */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, FTP_CONNECTION_TIMEOUT);

    do
    {
        /* Curl easy perform */
        performStatus = curl_easy_perform(curl);
        if (performStatus != CURLE_OK)
        {
            /* Get status based on curl perform response status */
            EPRINT(FTP_CLIENT, "fail to list dir: [path=%s], [performStatus=%d]", path, performStatus);
            retVal = getFtpRespFromCurlResp(performStatus);
            break;
        }

        /* Check FTP performance status */
        if (listInfo.retVal != FTP_SUCCESS)
        {
            EPRINT(FTP_CLIENT, "fail to perform list dir: [path=%s], [performStatus=%d]", path, performStatus);
            break;
        }

        /* If data available then provide in callback */
        if (listInfo.dataPtr != NULL)
        {
            callback(listInfo.dataPtr, listInfo.size, userData);
        }

        /* Successfully perform FTP operation */
        retVal = FTP_SUCCESS;

    }while(0);

    /* Free slist */
    if (list != NULL)
    {
        curl_slist_free_all(list);
    }

    /* Free allocated memory */
    FREE_MEMORY(listInfo.dataPtr);

    /* free curl handle */
    curl_easy_cleanup(curl);

	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function delete given directory / file from FTP
 * @param   ftpCfg
 * @param   sysType
 * @param   path
 * @param   isDir
 * @param   isEmpty
 * @return  FTP_RESPONSE_e
 */
FTP_RESPONSE_e DeleteFromFtp(FTP_UPLOAD_CONFIG_t *ftpCfg, FTP_SYS_e sysType, CHARPTR path, BOOL isDir, BOOL isEmpty)
{
    CURL                    *curl;
    CURLcode                performStatus;
    INT16                   outLen = 0;
    CHAR                    macAddress[MAX_MAC_ADDRESS_WIDTH] = { 0 };
    CHAR                    url[FTP_REMOTE_PATH_LEN];
    CHAR                    deleteCmd[FTP_REMOTE_PATH_LEN];
    struct curl_slist       *list = NULL;
    static const CHARPTR    ftpDelCmdStr[] = {"DELE", "RMD"};
    FTP_DEL_INFO_t          delInfo = {.ftpCfg = ftpCfg, .path = path, .retVal = FTP_SUCCESS, .sysType = sysType};
    CHAR                    ipAddrForUrl[DOMAIN_NAME_SIZE_MAX] = { 0 };

    if ((path == NULL) || (ftpCfg == NULL))
	{
        return FTP_FAIL;
    }

    DPRINT(FTP_CLIENT, "delete dir: [path=%s], [isDir=%d], [isEmpty=%d]", path, isDir, isEmpty);
    if ((isDir == TRUE) && (isEmpty == FALSE))
    {
        /* Delete All Sub-Folders & Files From Directory */
        FTP_RESPONSE_e retVal = ListFromFtp(ftpCfg, sysType, TRUE, path, &delInfo, delFileCb);
        if (retVal != FTP_SUCCESS)
        {
            return retVal;
        }

        if (delInfo.retVal != FTP_SUCCESS)
        {
            return delInfo.retVal;
        }
    }

    GetMacAddrPrepare(LAN1_PORT, macAddress);
    if (ftpCfg->uploadPath[0] != '\0')
    {
        outLen = snprintf(deleteCmd, FTP_REMOTE_PATH_LEN, "%s %s%c%s%c%s",
                          ftpDelCmdStr[isDir], ftpCfg->uploadPath, FTP_FILE_SEPERATOR, macAddress, FTP_FILE_SEPERATOR, path);
    }
    else
    {
        outLen = snprintf(deleteCmd, FTP_REMOTE_PATH_LEN, "%s %s%c%s", ftpDelCmdStr[isDir], macAddress, FTP_FILE_SEPERATOR, path);
    }

    /* Check if delete command size is more than our buffer size */
    if (outLen >= FTP_REMOTE_PATH_LEN)
    {
        EPRINT(FTP_CLIENT, "buffer is too small for remote path: [buffer=%d], [outLen=%d]", FTP_REMOTE_PATH_LEN, outLen);
        return FTP_FAIL;
    }

    // When Deleting directory Name must not end with '/'
    if (isDir == TRUE)
    {
        if (deleteCmd[outLen - 1] == '/')
        {
            deleteCmd[outLen - 1] = '\0';
        }
    }

    /* Create curl session */
    curl = curl_easy_init();
    if (curl == NULL)
    {
        EPRINT(FTP_CLIENT, "curl init fail");
        return FTP_FAIL;
    }

    /* Add delete file/dir command in header */
    list = curl_slist_append(list, deleteCmd);
    curl_easy_setopt(curl, CURLOPT_QUOTE, list);

    PrepareIpAddressForUrl(ftpCfg->server, ipAddrForUrl);

    /* Add curl URL in header */
    snprintf(url, sizeof(url), "ftp://%s:%d/", ipAddrForUrl, ftpCfg->serverPort);

    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* Add username of server */
    curl_easy_setopt(curl, CURLOPT_USERNAME, ftpCfg->username);

    /* Add password of server */
    curl_easy_setopt(curl, CURLOPT_PASSWORD, ftpCfg->password);

    /* Add option to inform curl to do not use signal */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    /* Do the download request without getting the body */
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

    /* TCP connection timeout */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, FTP_CONNECTION_TIMEOUT);

    /* Curl easy perform */
    performStatus = curl_easy_perform(curl);
    if (performStatus != CURLE_OK)
    {
        EPRINT(FTP_CLIENT, "fail to delete file: [path=%s], [performStatus=%d]", path, performStatus);
    }

    /* Free slist */
    if (list != NULL)
    {
        curl_slist_free_all(list);
    }

    /* free curl handle */
    curl_easy_cleanup(curl);

    /* Get status based on curl perform response status */
    return getFtpRespFromCurlResp(performStatus);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetFtpServerSystemType
 * @param   ftpCfg
 * @param   sysType
 * @return  FTP_RESPONSE_e
 */
FTP_RESPONSE_e GetFtpServerSystemType(FTP_UPLOAD_CONFIG_t *ftpCfg, FTP_SYS_e *sysType)
{
	CURL 					*curl;
	CURLcode 				performStatus;
    CHAR					url[FTP_REMOTE_PATH_LEN];
	struct curl_slist		*list = NULL;
	static const CHARPTR	systCmd = "SYST";
	static const CHARPTR	systTypeStr[MAX_FTP_SYS] = {"UNIX", "Windows_NT"};
    CHAR                    ipAddressForUrl[DOMAIN_NAME_SIZE_MAX];

    if (ftpCfg == NULL)
	{
        return FTP_FAIL;
    }

    /* Init with invalid value */
    *sysType = MAX_FTP_SYS;

    /* Create curl session */
    curl = curl_easy_init();
    if (curl == NULL)
    {
        return FTP_FAIL;
    }

    /* Add system type in header */
    list = curl_slist_append(list, systCmd);
    curl_easy_setopt(curl, CURLOPT_QUOTE, list);

    /* Add option to inform curl to do not use signal */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    PrepareIpAddressForUrl(ftpCfg->server, ipAddressForUrl);

    /* Add curl URL in header */
    snprintf(url, FTP_REMOTE_PATH_LEN, "ftp://%s:%d/", ipAddressForUrl, ftpCfg->serverPort);

    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* Add username and password in header */
    /* Add username of server */
    curl_easy_setopt(curl, CURLOPT_USERNAME, ftpCfg->username);

    /* Add password of server */
    curl_easy_setopt(curl, CURLOPT_PASSWORD, ftpCfg->password);

    /* Set callback function */
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curlVerboseCb);

    /* Pass pointer to fetch system type */
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, sysType);

    /* Do the download request without getting the body */
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

    /* TCP connection timeout */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, FTP_CONNECTION_TIMEOUT);

    /* Curl easy perform */
    performStatus = curl_easy_perform(curl);
    if (performStatus != CURLE_OK)
    {
        EPRINT(FTP_CLIENT, "fail to get remote system type: [ftpServer=%s], [performStatus=%d]", ftpCfg->server, performStatus);
    }
    else if (*sysType < MAX_FTP_SYS)
    {
        DPRINT(FTP_CLIENT, "ftp success: [ftpServer=%s], [systemType=%s]", ftpCfg->server, systTypeStr[*sysType]);
    }

    /* free slist */
    if (NULL != list)
    {
        curl_slist_free_all(list);
    }

    /* free curl handle */
    curl_easy_cleanup(curl);

    /* Get status based on curl perform response status */
    return getFtpRespFromCurlResp(performStatus);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get FTP status in string
 * @param   ftpStatus
 * @return  FTP status in string
 */
const CHARPTR GetFtpStatusStr(FTP_RESPONSE_e ftpStatus)
{
    switch(ftpStatus)
    {
        case FTP_SUCCESS:
            return "Success";

        case FTP_CONNECTION_ERROR:
            return "Connection error";

        case FTP_AUTH_ERROR:
            return "Authentication failed";

        case FTP_USER_ABORTED:
            return "User has aborted";

        case FTP_WRITE_PROTECTED:
            return "Write protected";

        case FTP_QUEUE_FULL:
            return "Internal queue full";

        case FTP_FAIL:
        default:
            return "Unknown";
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   imageUploadThread
 * @param   threadArg
 * @return
 */
static VOIDPTR imageUploadThread(VOIDPTR threadArg)
{
    FTP_IMG_UPLD_t		*ftpImgUpldPtr = (FTP_IMG_UPLD_t *)threadArg;
    FTP_REQUEST_LIST_t	*reqPtr = NULL;
    FTP_REQUEST_LIST_t	*tmpReqPtr = NULL;
	FTP_RESPONSE_e		retVal;
    CHAR                eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR                advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];
    UINT8               retryCnt = 0;

	THREAD_START_INDEX("FTP_IMG_UPLD", ftpImgUpldPtr->ftpIdx);

    while (TRUE)
	{
		retVal = FTP_FAIL;
		if (reqPtr == NULL)
		{
            reqPtr = (FTP_REQUEST_LIST_t *)QueueGetAndFreeEntry(ftpImgUpldPtr->ftpQHandle, TRUE);
            if (reqPtr == NULL)
            {
                continue;
            }
		}
		else
		{
            tmpReqPtr = (FTP_REQUEST_LIST_t *)QueueGetNewEntryIfOverWritten(ftpImgUpldPtr->ftpQHandle);
			if (tmpReqPtr != NULL)
			{
				unlink(reqPtr->ftpInfo.localFileName);
				free(reqPtr);
				reqPtr = tmpReqPtr;
			}
		}

		retVal = ftpTransfer(reqPtr);

		if (retVal == FTP_CONNECTION_ERROR)
        {
            retryCnt++;
            if (retryCnt < MAX_FTP_CONN_RETRY)
            {
                sleep(FTP_CONN_REFRESH_TIME);
                continue;
            }
        }

        retryCnt = 0;

		if (retVal != FTP_SUCCESS)
		{
            snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(reqPtr->userData));
            snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "FTP-%d | %s", ftpImgUpldPtr->ftpIdx+1, GetFtpStatusStr(retVal));
            WriteEvent(LOG_NETWORK_EVENT, LOG_UPLOAD_IMAGE, eventDetail, advncDetail, EVENT_FAIL);
		}

		unlink(reqPtr->ftpInfo.localFileName);
        FREE_MEMORY(reqPtr);
	}

	pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ftpThread
 * @param   threadArg
 * @return
 */
static VOIDPTR ftpThread(VOIDPTR threadArg)
{
	FTP_REQUEST_LIST_t *sessionPtr = (FTP_REQUEST_LIST_t *)threadArg;

    if(sessionPtr != NULL)
    {
        ftpTransfer(sessionPtr);
        MUTEX_LOCK(ftpDataMutex);
        sessionPtr->requestStatus = FREE;
        MUTEX_UNLOCK(ftpDataMutex);
    }

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is thread entry function that perform data transfer to/from FTP server.
 *          First curl parameters are set and then actual data transfer is performed. After performing
 *          transfer, user registered callback function is called indicating status of requested transfer.
 * @param   sessionPtr
 * @return
 */
static FTP_RESPONSE_e ftpTransfer(FTP_REQUEST_LIST_t *sessionPtr)
{
    struct curl_slist 	*headerList = NULL;
	struct stat 		fileSizeInfo;
    FILE				*fileHandle;
	CURLcode 			performStatus = CURLE_OK;
	FTP_RESPONSE_e		retVal = FTP_FAIL;

    if(sessionPtr == NULL)
    {
        EPRINT(FTP_CLIENT, "invld input params");
        return FTP_FAIL;
    }

	do
	{
        /* Create curl session */
		sessionPtr->curlHandle = curl_easy_init();
		if(sessionPtr->curlHandle == NULL)
		{
            EPRINT(FTP_CLIENT, "fail to init curl: [session=%u]", sessionPtr->ftpSessionHandle);
			break;
		}

        /* Inform curl to stop all progress meter */
        curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_NOPROGRESS, 0L);

        /* Set progress meter callback */
        curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_PROGRESSFUNCTION, abortFtpTransfer);

        /* Pointer passed to the progree callback */
        curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_PROGRESSDATA, &sessionPtr->ftpSessionHandle);

        /* Add option to inform curl to do not use signal */
        curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_NOSIGNAL, 1L);

        /* Add username of server */
        curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_USERNAME, sessionPtr->userName);

        /* Add password of server */
        curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_PASSWORD, sessionPtr->password);

        /* Use default auth for FTP */
        curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_FTPSSLAUTH, CURLFTPAUTH_DEFAULT);

        /* TCP connection timeout */
        curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_CONNECTTIMEOUT, FTP_CONNECTION_TIMEOUT);

        /* Low speed limit in bytes per second */
        curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_LOW_SPEED_LIMIT, 10L);

        /* Low speed limit time period */
        curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_LOW_SPEED_TIME, 20L);

        /* Set option to upload the file on FTP */
		if(sessionPtr->requestType == FTP_UPLOAD)
		{
            /* Enable data upload */
            curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_UPLOAD, 1L);

            /* Create missing directories */
            curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

            /* Command to run after the transfer */
            curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_POSTQUOTE, headerList);

            /* Get file size */
            if(stat(sessionPtr->ftpInfo.localFileName, &fileSizeInfo) != STATUS_OK)
			{
                EPRINT(FTP_CLIENT, "file not present: [session=%d], [path=%s]", sessionPtr->ftpSessionHandle, sessionPtr->ftpInfo.localFileName);
				break;
			}

            /* Open file which is to be upload */
			fileHandle = fopen(sessionPtr->ftpInfo.localFileName, "rb");
			if(fileHandle == NULL)
			{
                EPRINT(FTP_CLIENT, "fail to open file for read: [session=%d], [path=%s]", sessionPtr->ftpSessionHandle, sessionPtr->ftpInfo.localFileName);
				break;
			}

            /* Pointer passed to the read callback. If you do not specify a read callback but instead rely on the default
             * internal read function, this data must be a valid readable FILE * (cast to 'void *'). */
            curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_READDATA, fileHandle);

            /* Size of the input file to send */
            curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fileSizeInfo.st_size);
		}
        else	/* Set option to download the file from FTP */
		{
            /* Open file which is to be download */
			fileHandle = fopen(sessionPtr->ftpInfo.localFileName, "wb");
			if(fileHandle == NULL)
			{
                EPRINT(FTP_CLIENT, "fail to open file for write: [session=%d], [path=%s]", sessionPtr->ftpSessionHandle, sessionPtr->ftpInfo.localFileName);
				break;
			}

            /* Pointer passed to the write callback. If you do not use a write callback, you must make pointer a 'FILE *'
             * (cast to 'void *') as libcurl will pass this to fwrite when writing data. */
            curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_WRITEDATA, fileHandle);
		}

        /* Add curl URL in header */
        curl_easy_setopt(sessionPtr->curlHandle, CURLOPT_URL, sessionPtr->ftpInfo.remoteFile);

        /* Curl easy perform */
		performStatus = curl_easy_perform(sessionPtr->curlHandle);
		if(performStatus != CURLE_OK)
		{
            EPRINT(FTP_CLIENT, "ftp operation fail: [session=%d], [performStatus=%d]", sessionPtr->ftpSessionHandle, performStatus);
        }

        /* Get status based on curl perform response status */
        retVal = getFtpRespFromCurlResp(performStatus);

        /* Close the file */
		fclose(fileHandle);

    } while(0);

    /* Send response to user for relative request */
	if(sessionPtr->userCallback != NULL)
	{
		(sessionPtr->userCallback)(sessionPtr->ftpSessionHandle, retVal, sessionPtr->userData);
		sessionPtr->userCallback = NULL;
	}

    /* Cleanup curl handle */
	if(sessionPtr->curlHandle != NULL)
	{
		curl_easy_cleanup(sessionPtr->curlHandle);
		sessionPtr->curlHandle = NULL;
	}

	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is curl's callback funtion on progress status. From this function we can
 *          terminate our ftp transfer by just returning to non Zero value.
 * @param   clientp
 * @param   dltotal
 * @param   dlnow
 * @param   ultotal
 * @param   ulnow
 * @return
 */
static INT32 abortFtpTransfer(VOIDPTR clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    FTP_HANDLE  ftpHandle = *(FTP_HANDLE *)clientp;
    UINT8       retStatus = 0;

    MUTEX_LOCK(ftpDataMutex);
    if(ftpRequestList[ftpHandle].terminateFlg == TRUE)
	{
		retStatus = 1;
	}
    MUTEX_UNLOCK(ftpDataMutex);
	return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is call back for ftp upload test invoked by ftp upload api of FTP client. In this
 *          function we received actual status of test upload.
 * @param   ftpHandle
 * @param   ftpResponse
 * @param   userData
 */
static void testFtpConnCallback(FTP_HANDLE ftpHandle, FTP_RESPONSE_e ftpResponse, UINT16 userData)
{
    NET_CMD_STATUS_e retVal = CMD_TESTING_FAIL;

	if(ftpResponse == FTP_SUCCESS)
	{
        DPRINT(FTP_CLIENT, "ftp server connection test success: [ftpHandle=%d]", ftpHandle);
        retVal = CMD_SUCCESS;
	}
	else
	{
		if(ftpResponse == FTP_AUTH_ERROR)
		{
            retVal = CMD_FTP_INVALID_CREDENTIAL;
		}
		else if(ftpResponse == FTP_CONNECTION_ERROR)
		{
            retVal = CMD_CONNECTIVITY_ERROR;
		}
		else if(ftpResponse == FTP_WRITE_PROTECTED)
		{
            retVal = CMD_NO_WRITE_PERMISSION;
		}
        EPRINT(FTP_CLIENT, "ftp server test connection fail: [ftpHandle=%d], [status=%d]", ftpHandle, retVal);
	}

	if (ftpConnTest.callback != NULL)
	{
        ftpConnTest.callback(retVal, ftpConnTest.connFd, TRUE);
		ftpConnTest.callback = NULL;
	}

	ftpConnTest.connFd = INVALID_CONNECTION;
	unlink(ftpConnTest.localFileName);

    MUTEX_LOCK(ftpConnTest.runStatusMutex);
	ftpConnTest.runStatus = FREE;
    MUTEX_UNLOCK(ftpConnTest.runStatusMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function copies data from curl buffer to local buffer, we need this extra copy because
 *          we want FTP file Deletion Recursive, and curl closes sockets after this callback, Recursive
 *          function call in this case will lead to so many open socket file descriptors,so we can't
 *          give callback to user from here, to have access to this data we must store it into local buffer.
 * @param   buffer
 * @param   row
 * @param   column
 * @param   stream
 * @return
 */
static size_t curlListCb(VOIDPTR buffer, size_t row, size_t column, VOIDPTR stream)
{
    UINT32 length = (row * column);

    if ((buffer == NULL) || (length == 0))
	{
        return length;
    }

    FTP_LIST_INFO_t *listInfo = (FTP_LIST_INFO_t*)stream;
    listInfo->dataPtr = malloc(length + 1);
    if (listInfo->dataPtr == NULL)
    {
        listInfo->retVal = FTP_FAIL;
        return length;
    }

    memcpy(listInfo->dataPtr, buffer, length);
    listInfo->dataPtr[length] = '\0';
    listInfo->size = length;
	return length;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback Function to ListFromFtp API, It deletes all files in list.
 * @param   list
 * @param   size
 * @param   userData
 * @return
 */
static BOOL delFileCb(CHARPTR list, UINT32 size, VOIDPTR userData)
{
    FTP_DEL_INFO_t	*delInfo = (FTP_DEL_INFO_t	*)userData;
	CHARPTR			tmpListPtr = list;
	CHAR			entry[FTP_REMOTE_PATH_LEN];
	BOOL			isDir;
	UINT16			outLen;

    if (size == 0)
	{
        return SUCCESS;
    }

    snprintf(entry, FTP_REMOTE_PATH_LEN, "%s", delInfo->path);
    outLen = strlen(entry);

    while ((tmpListPtr - list) < (INT32)size)
    {
        /* Parse till all entry are resolved */
        if (ParseFTPListReply(&tmpListPtr, (entry + outLen), FTP_REMOTE_PATH_LEN - outLen, &isDir) != SUCCESS)
        {
            delInfo->retVal = FTP_FAIL;
            break;
        }

        delInfo->retVal = DeleteFromFtp(delInfo->ftpCfg, delInfo->sysType, entry, isDir, FALSE);
        if (delInfo->retVal != FTP_SUCCESS)
        {
            break;
        }
    }

	return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is callback Function to curl for debug.
 * @param   buffer
 * @param   row
 * @param   column
 * @param   stream
 * @return
 */
static size_t curlVerboseCb(VOIDPTR buffer, size_t row, size_t column, VOIDPTR stream)
{
	CHARPTR		respPtr = (CHARPTR)buffer;
	FTP_SYS_e	*systemType = (FTP_SYS_e *)stream;

	// Check if this is response for System Name
	if (strstr(respPtr, "215") != NULL)
	{
		if (strstr(respPtr, "UNIX") != NULL)
		{
			*systemType = FTP_SYS_UNIX;
		}
		else if (strstr(respPtr, "Windows_NT") != NULL)
		{
			*systemType = FTP_SYS_WINDOWS_NT;
		}
	}

	return (row * column);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getFtpRespFromCurlResp
 * @param   performStatus
 * @return
 */
static FTP_RESPONSE_e getFtpRespFromCurlResp(CURLcode performStatus)
{
    switch(performStatus)
    {
        case CURLE_OK:
            return FTP_SUCCESS;

        case CURLE_COULDNT_CONNECT:
            return FTP_CONNECTION_ERROR;

        case CURLE_LOGIN_DENIED:
            return FTP_AUTH_ERROR;

        case CURLE_ABORTED_BY_CALLBACK:
            return FTP_USER_ABORTED;

        case CURLE_REMOTE_ACCESS_DENIED:
            return FTP_WRITE_PROTECTED;

        default:
            return FTP_FAIL;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This callback is called when queue is full, and the entry is being over written
 * @param   entry
 */
static void queueFullCb(VOIDPTR entry)
{
	FTP_REQUEST_LIST_t	*reqPtr = (FTP_REQUEST_LIST_t *)entry;
    CHAR                eventDetail[MAX_EVENT_DETAIL_SIZE];
    CHAR                advncDetail[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE, "%02d", GET_CAMERA_NO(reqPtr->userData));
    snprintf(advncDetail, MAX_EVENT_ADVANCE_DETAIL_SIZE, "FTP-%d | %s", reqPtr->ftpInfo.ftpServer+1, GetFtpStatusStr(FTP_QUEUE_FULL));
    WriteEvent(LOG_NETWORK_EVENT, LOG_UPLOAD_IMAGE, eventDetail, advncDetail, EVENT_FAIL);
	unlink(reqPtr->ftpInfo.localFileName);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
