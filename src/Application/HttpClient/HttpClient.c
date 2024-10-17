
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		HttpClient.h
@brief      This module provides communication functionality over http protocol
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "HttpClient.h"
#include "DebugLog.h"
#include "Utils.h"
#include "MxHttpParser.h"
#include "NetworkInterface.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define POSTDATA_DELIM				("?")
#define MAX_URL_WIDTH				1024
#define NS_HTTP_USER_AGENT_STR      "NS-HTTP/1.0"

/* Use Default Stack Size*/
#define HTTP_TRANSFER_STACK_SZ      (1*MEGA_BYTE)

//#################################################################################################
// @DATA_TYPES
//#################################################################################################
typedef struct
{
	BOOL 					stopHttp;			// flag to stop HTTP request
	BOOL		 			requestStatus;
	HTTP_HANDLE				httpHandle;
	HTTP_REQUEST_e 			httpRequest;		// type of HTTP request
    CURL 					*curlHandle;
	HTTP_INFO_t 			httpInfo;			// URL information buffer
	pthread_mutex_t 		httpReqInfoMutex;	// Stop HTTP lock mutex
}HTTP_CLIENT_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static pthread_mutex_t      httpReqListMutex;
static HTTP_CLIENT_INFO_t   httpClientInfo[MAX_HTTP_REQUEST];

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static VOIDPTR httpTransfer(VOIDPTR httpSessionPtr);
//-------------------------------------------------------------------------------------------------
static size_t httpHeaderRead(VOIDPTR curlBuffer, size_t row, size_t column, VOIDPTR index);
//-------------------------------------------------------------------------------------------------
static size_t httpDataRead(VOIDPTR curlBuffer, size_t row, size_t column, VOIDPTR index);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API Initializes HTTP module.
 */
void InitHttp(void)
{
    HTTP_HANDLE handle;

    MUTEX_INIT(httpReqListMutex, NULL);

	// traverse through request array, and default each element
    for(handle = 0; handle < MAX_HTTP_REQUEST; handle++)
	{
        httpClientInfo[handle].requestStatus = FREE;
        httpClientInfo[handle].httpHandle = handle;
        MUTEX_INIT(httpClientInfo[handle].httpReqInfoMutex, NULL);
	}

	// initialize LibCurl
	curl_global_init(CURL_GLOBAL_ALL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API starts communication with HTTP server
 * @param   httpRequest - request type
 * @param   httpInfo - url information
 * @param   callback - callback function pointer
 * @param   userData - user data
 * @param   handlePtr - handle to HTTP request
 * @return  SUCCESS or FAIL
 */
BOOL StartHttp(HTTP_REQUEST_e httpRequest, HTTP_INFO_t * httpInfo, HTTP_CALLBACK callback, UINT32 userData, HTTP_HANDLE *handlePtr)
{
    HTTP_HANDLE handle;

	// check input/output parameters are not invalid, if any of them is, return fail status
    if ((httpRequest >= MAX_HTTP_REQUEST_TYPE) || (callback == NULL))
	{
        EPRINT(HTTP_CLIENT, "invld input param: [httpRequest=%d], [callback=%p]", httpRequest, callback);
        return FAIL;
    }

    /* Check if the space for new request is available or not. lock the request list prior to search operation.
     * so that another thread/process can’t alter content which we are accessing. Check request status from the
     * beginning, until free space is found or end of list is reached */
    MUTEX_LOCK(httpReqListMutex);
    for(handle = 0; handle < MAX_HTTP_REQUEST; handle++)
    {
        if(httpClientInfo[handle].requestStatus == FREE)
        {
            httpClientInfo[handle].requestStatus = BUSY;
            break;
        }
    }
    MUTEX_UNLOCK(httpReqListMutex);

    // if pointer has reached MAX_HTTP_REQUEST, no space is available
    if (handle >= MAX_HTTP_REQUEST)
    {
        EPRINT(HTTP_CLIENT, "free http session not available: [httpRequest=%d]", httpRequest);
        return FAIL;
    }

    httpClientInfo[handle].httpRequest = httpRequest;
    httpClientInfo[handle].curlHandle = NULL;
    httpClientInfo[handle].httpInfo = *httpInfo;
    httpClientInfo[handle].stopHttp = FALSE;

    InitHttpParser(handle, callback, userData);

    /* Create HTTP parser thread */
    if (FALSE == Utils_CreateThread(NULL, httpTransfer, &httpClientInfo[handle], DETACHED_THREAD, HTTP_TRANSFER_STACK_SZ))
    {
        MUTEX_LOCK(httpReqListMutex);
        httpClientInfo[handle].requestStatus = FREE;
        MUTEX_UNLOCK(httpReqListMutex);
        *handlePtr = INVALID_HTTP_HANDLE;
        return FAIL;
    }

    *handlePtr = handle;
    DPRINT(HTTP_CLIENT, "http session started: [handle=%d]", handle);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API terminates communication with HTTP server
 * @param   handle to HTTP request
 * @return  SUCCESS or FAIL
 */
BOOL StopHttp(HTTP_HANDLE httpHandle)
{
    if (httpHandle >= MAX_HTTP_REQUEST)
	{
        EPRINT(HTTP_CLIENT, "invld http handle: [handle=%d]", httpHandle);
        return FAIL;
    }

    MUTEX_LOCK(httpReqListMutex);
    if ((httpClientInfo[httpHandle].httpHandle != httpHandle) || (httpClientInfo[httpHandle].requestStatus != BUSY))
    {
        MUTEX_UNLOCK(httpReqListMutex);
        EPRINT(HTTP_CLIENT, "http session is already free: [handle=%d]", httpHandle);
        return FAIL;
    }
    MUTEX_UNLOCK(httpReqListMutex);

    /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
    MUTEX_LOCK(httpClientInfo[httpHandle].httpReqInfoMutex);
    httpClientInfo[httpHandle].stopHttp = TRUE;
    MUTEX_UNLOCK(httpClientInfo[httpHandle].httpReqInfoMutex);
    DPRINT(HTTP_CLIENT, "http session stop: [handle=%d]", httpHandle);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a thread function. It performs actual communication with server
 * @param   httpSessionPtr - http handle
 * @return  NULL
 */
static VOIDPTR httpTransfer(VOIDPTR httpSessionPtr)
{
	HTTP_HANDLE 		httpHandle;
    UINT32				authType;
	HTTP_RESPONSE_e		httpResp = HTTP_CLOSE_ON_ERROR;
    CURLcode 			curlResp = CURLE_OK;
	CHAR 				absoluteUrl[MAX_URL_WIDTH];
	CHARPTR 			postUrl, postData;
	FILE 				*putFileFd = NULL;
    struct curl_slist   *httpHeader = NULL;
    CHAR                ipAddrForUrl[DOMAIN_NAME_SIZE_MAX] = { 0 };

	httpHandle = ((HTTP_CLIENT_INFO_t *)httpSessionPtr)->httpHandle;
    if (httpHandle >= MAX_HTTP_REQUEST)
	{
        DPRINT(HTTP_CLIENT, "invld http handle: [handle=%d]", httpHandle);
        pthread_exit(NULL);
    }

    THREAD_START_INDEX2("HTTP_CLT", httpHandle);

    /* Compose URL from the user provided data to be feed to libcurl later */
    PrepareIpAddressForUrl(httpClientInfo[httpHandle].httpInfo.ipAddress, ipAddrForUrl);
    snprintf(absoluteUrl, MAX_URL_WIDTH, "%s://%s:%d%s", (httpClientInfo[httpHandle].httpInfo.port == DFLT_HTTPS_PORT) ? "https" : "http",
             ipAddrForUrl, httpClientInfo[httpHandle].httpInfo.port, httpClientInfo[httpHandle].httpInfo.relativeUrl);

    do
    {
        // initialize curl handle, which will be used by LibCurl during communication
        httpClientInfo[httpHandle].curlHandle = curl_easy_init();
        if (NULL == httpClientInfo[httpHandle].curlHandle)
        {
            EPRINT(HTTP_CLIENT, "failed to init curl: [handle=%d], [url=%s]", httpHandle, absoluteUrl);
            break;
        }

        // configure behavior option
        curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_NOSIGNAL, 1L);

        // Http version
        curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_NONE);

        // User agent
        if (httpClientInfo[httpHandle].httpInfo.userAgent == NS_HTTP_USER_AGENT)
        {
            curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_USERAGENT, NS_HTTP_USER_AGENT_STR);
        }
        else
        {
            curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_USERAGENT, curl_version());
        }

        // configure callback options
        curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_HEADERFUNCTION, httpHeaderRead);
        curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_HEADERDATA, &httpHandle);

        /* SSL options: disable peer verification */
        curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_SSL_VERIFYHOST, 0L);

        if (httpClientInfo[httpHandle].httpRequest == GET_REQUEST || httpClientInfo[httpHandle].httpRequest == POST_REQUEST)
        {
            curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_WRITEFUNCTION, httpDataRead);
            curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_WRITEDATA, &httpHandle);
        }
        else if(httpClientInfo[httpHandle].httpRequest == PUT_REQUEST)
        {
            /* if content type is xml */
            if (httpClientInfo[httpHandle].httpInfo.contentType == HTTP_CONTENT_TYPE_XML)
            {
                /* append content type in http header, added for tiandy camera support */
                httpHeader = curl_slist_append(httpHeader, "Content-Type: application/xml");
                curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_HTTPHEADER, httpHeader);
            }

            if(httpClientInfo[httpHandle].httpInfo.sizeOfPutFile == 0)
            {
                curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)(httpClientInfo[httpHandle].httpInfo.sizeOfPutFile));

                /* set the METHOD put */
                curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_PUT, 1L);
                curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_WRITEFUNCTION, httpDataRead);
                curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_WRITEDATA, &httpHandle);
            }
            else
            {
                putFileFd = fopen(httpClientInfo[httpHandle].httpInfo.fileForPutReq, "r");
                if(putFileFd == NULL)
                {
                    EPRINT(HTTP_CLIENT, "failed to open file for put request: [handle=%d], [file=%s], [url=%s]",
                           httpHandle, httpClientInfo[httpHandle].httpInfo.fileForPutReq, absoluteUrl);
                    break;
                }

                /* enable uploading */
                curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_UPLOAD, 1L);

                /* now specify which file to upload */
                curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_READDATA, putFileFd);

                /* provide the size of the upload, we specicially typecast the value
                 * to curl_off_t since we must be sure to use the correct data size */
                curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)(httpClientInfo[httpHandle].httpInfo.sizeOfPutFile));

                /* set the METHOD put */
                curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_PUT, 1L);
                curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_WRITEFUNCTION, httpDataRead);
                curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_WRITEDATA, &httpHandle);
            }
        }
        /* This type is for REBOOT the camera where we dont require any file to open. hikvision supports reboot of camera
         * on PUT method where we dont have to give any file and file size */
        else if(httpClientInfo[httpHandle].httpRequest == PUT_REBOOT_REQUEST)
        {
            curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_WRITEFUNCTION, httpDataRead);
            curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_WRITEDATA, &httpHandle);
        }
        else if(httpClientInfo[httpHandle].httpRequest == DELETE_REQUEST)
        {
            curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_WRITEFUNCTION, httpDataRead);
            curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_WRITEDATA, &httpHandle);
        }

        // configure authentication options
        curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_USERNAME, httpClientInfo[httpHandle].httpInfo.httpUsrPwd.username);
        curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_PASSWORD, httpClientInfo[httpHandle].httpInfo.httpUsrPwd.password);

        switch(httpClientInfo[httpHandle].httpInfo.authMethod)
        {
            case AUTH_TYPE_NONE:
                authType = CURLAUTH_NONE;
                break;

            case AUTH_TYPE_BASIC:
                authType = CURLAUTH_BASIC;
                break;

            case AUTH_TYPE_DIGEST:
                authType = CURLAUTH_DIGEST;
                break;

            default:
                authType = (UINT32)CURLAUTH_ANY;
                break;
        }

        curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_HTTPAUTH, authType);

        // configure HTTP options
        curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_CONNECTTIMEOUT, httpClientInfo[httpHandle].httpInfo.maxConnTime);
        curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_LOW_SPEED_TIME, httpClientInfo[httpHandle].httpInfo.maxFrameTime);

        // configure Connection options
        curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_LOW_SPEED_LIMIT, 1);

        // configure parameters special to GET request
        if(httpClientInfo[httpHandle].httpRequest == GET_REQUEST || httpClientInfo[httpHandle].httpRequest == PUT_REQUEST
                || httpClientInfo[httpHandle].httpRequest == PUT_REBOOT_REQUEST || httpClientInfo[httpHandle].httpRequest == DELETE_REQUEST)
        {
            // configure network options
            curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_URL, absoluteUrl);
        }
        // configure parameters special for POST request
        else
        {
            // separate out POST data from URL
            postUrl = strtok_r(absoluteUrl, POSTDATA_DELIM, &postData);
            curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_URL, postUrl);
            curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_POSTFIELDS, postData);
        }

        if (httpClientInfo[httpHandle].httpInfo.interface == INTERFACE_USB_MODEM)
        {
            CHAR ifaceName[INTERFACE_NAME_LEN_MAX];

            GetNetworkPortName(NETWORK_PORT_USB_MODEM, ifaceName);
            if (ifaceName[0] != '\0')
            {
                curl_easy_setopt(httpClientInfo[httpHandle].curlHandle, CURLOPT_INTERFACE, ifaceName);
            }
        }

        curlResp = curl_easy_perform(httpClientInfo[httpHandle].curlHandle);

        // If curl exit normally or stop is requested, return CLOSE_WITH_SUCCESS status in all other cases
        MUTEX_LOCK(httpClientInfo[httpHandle].httpReqInfoMutex);
        if ((curlResp == CURLE_OK) || ((curlResp == CURLE_WRITE_ERROR) && (httpClientInfo[httpHandle].stopHttp == TRUE)))
        {
            MUTEX_UNLOCK(httpClientInfo[httpHandle].httpReqInfoMutex);
            httpResp = HTTP_CLOSE_ON_SUCCESS;
        }
        else
        {
            MUTEX_UNLOCK(httpClientInfo[httpHandle].httpReqInfoMutex);
            EPRINT(HTTP_CLIENT, "http failed: [handle=%d], [stopHttp=%d], [curlResp=%d], [url=%s]",
                   httpHandle, httpClientInfo[httpHandle].stopHttp, curlResp, absoluteUrl);
        }

        if((httpClientInfo[httpHandle].httpRequest == PUT_REQUEST) && (httpClientInfo[httpHandle].httpInfo.sizeOfPutFile != 0) && (putFileFd != NULL))
        {
            fclose(putFileFd);
            if(unlink(httpClientInfo[httpHandle].httpInfo.fileForPutReq) != STATUS_OK)
            {
                EPRINT(HTTP_CLIENT, "failed to remove put file: [handle=%d], [file=%s], [url=%s], [err=%s]",
                       httpHandle, httpClientInfo[httpHandle].httpInfo.fileForPutReq, absoluteUrl, STR_ERR);
            }
        }

    }
    while(0);

    curl_easy_cleanup(httpClientInfo[httpHandle].curlHandle);

    if (httpHeader != NULL)
    {
        /* free linked list of header strings */
        curl_slist_free_all(httpHeader);
    }

    CleanupHttpInfo(httpHandle, httpResp);

    // set http request buffer of current request to default
    MUTEX_LOCK(httpReqListMutex);
    httpClientInfo[httpHandle].requestStatus = FREE;
    MUTEX_UNLOCK(httpReqListMutex);
    if (curlResp == CURLE_OK)
    {
        DPRINT(HTTP_CLIENT, "http session done: [handle=%d], [url=%s]", httpHandle, absoluteUrl);
    }
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API is used as callback from curl. whenever header is received from server, curl invokes this API.
 * @param   curlBuffer - pointer to buffer, which contains incoming data
 * @param   row - length of member
 * @param   column - number of members
 * @param   index - http handle
 * @return  number of data read
 */
static size_t httpHeaderRead(VOIDPTR curlBuffer, size_t row, size_t column, VOIDPTR index)
{
	BOOL				stopHttp;
	size_t 				sizeToReturn = 0;
	HTTP_HANDLE 		httpHandle = *(HTTP_HANDLE *) index;

    MUTEX_LOCK(httpClientInfo[httpHandle].httpReqInfoMutex);
	stopHttp = httpClientInfo[httpHandle].stopHttp;
    MUTEX_UNLOCK(httpClientInfo[httpHandle].httpReqInfoMutex);

	if(stopHttp == FALSE)
	{
		sizeToReturn = (size_t)httpParseHeader(curlBuffer, row, column, httpHandle);
	}
	// return the number of bytes read
	return sizeToReturn;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API is used as callback from curl. whenever body is received from server, curl invokes this API.
 * @param   curlBuffer - pointer to buffer, which contains incoming data
 * @param   row - length of member
 * @param   column - number of members
 * @param   index - http handle
 * @return  number of data read
 */
static size_t httpDataRead(VOIDPTR curlBuffer, size_t row, size_t column, VOIDPTR index)
{
	BOOL				stopHttp;
	UINT32 				readPacket = 0;
	HTTP_HANDLE 		httpHandle = *(HTTP_HANDLE *)index;

    MUTEX_LOCK(httpClientInfo[httpHandle].httpReqInfoMutex);
	stopHttp = httpClientInfo[httpHandle].stopHttp;
    MUTEX_UNLOCK(httpClientInfo[httpHandle].httpReqInfoMutex);

	if(stopHttp == FALSE)
	{
		readPacket = (size_t)httpParseData(curlBuffer, row, column, httpHandle);
	}
	// return number of bytes read
	return readPacket;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Provides http authentication type
 * @param   httpHandle
 * @return  http authentication type
 */
REQUEST_AUTH_TYPE_e GetAuthType(HTTP_HANDLE httpHandle)
{
	return(httpClientInfo[httpHandle].httpInfo.authMethod);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
