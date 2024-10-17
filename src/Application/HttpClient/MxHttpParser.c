//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		MxHttpParser.h
@brief      This module provides http header and data parsing
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "MxHttpParser.h"
#include "DebugLog.h"
#include "CameraEvent.h"
#include "Utils.h"
#include "HttpClient.h"
#include "VideoParser.h"
#include "AudioParser.h"
#include "MxMp2TsParser.h"
#include "ConfigApi.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define HTTP_RESPONSE					"HTTP"
#define CONTENT_LENGTH_TAG1				"Content-Length"
#define CONTENT_LENGTH_TAG2				"DataLen"
#define CONTENT_TYPE_TAG				"Content-Type"
#define	TRANSFER_ENCODE_TAG				"Transfer-Encoding"
#define	CHUNK_TAG						"chunked"
#define BOUNDARY_TAG					"boundary"
#define MULTIPART_TAG					"multipart"
#define CR_LF_STR						"\r\n"
#define START_OF_BOUNDARY				"--"
#define HTTP_UNAUTHENTIC				401

#define HTTP_VIDEO_STREAM				"video"
#define HTTP_IMAGE_STREAM				"image"
#define HTTP_AUDIO_STREAM				"audio"
#define HTTP_TEXT_STREAM				"text"
#define HTTP_APPLICATION_STREAM			"application"

#define AUDIO_FRAME_CONFIG				"config="
#define MAX_MULTIPART_HDR_LEN			1024
#define	MAX_BOUNDARY_NAME_LEN			64
#define	MAX_AUDIO_CFG_LEN				12

#define	AAC_AUDIO_HDR_LEN				4
#define	PCM_AUDIO_HDR_LEN				28

#define MAX_HTTP_RESP_MAP_ENTRY			2

//#################################################################################################
// @DATA_TYPES
//#################################################################################################
typedef enum
{
	CONTENT_LENGTH_TAG_1 = 0,
	CONTENT_LENGTH_TAG_2,
	CONTENT_LENGTH_TAG_MAX,
}CONTENT_LENGTH_TAG_e;

typedef enum
{
	MULTI_PART_DATA = 0,	// Data will come continuously in multipart
	SINGLE_PART_DATA,		// Only data will be continuously received
	SINGLE_PART_IMAGE_DATA,	// Data continuously received having jpeg image
	SINGLE_RESPONSE_DATA,	// Defined size of data will be received and then connection will be closed
	MULTI_CHUNK_DATA,
	UNKNOWN_DATA_TYPE,
	MAX_HTTP_DATA_TYPE
}HTTP_DATA_TYPE_e;

typedef enum
{
	MULTIPART_PARSE_HEADER_STATE = 0,
	MULTIPART_GET_FRAME_STATE,
	MAX_MULTIPART_STATE
}HTTP_DATA_STATE_e;

typedef struct
{
	BOOL				dataParseStatus;
	BOOL				dataProcF;
	CHARPTR				readPtr;
	UINT32				parsedBytes;
}HTTP_STATE_INFO_t;

typedef struct
{
    BOOL 				isAuthReqSent;      // Authentication process completion flag
	UINT8				configDoneBit;		// Configuration is received or not BIT:0 - Video config, BIT:1 - Audio config
	HTTP_DATA_TYPE_e 	httpDataType;		// HTTP data type
	HTTP_DATA_STATE_e	httpDataState;		// HTTP data state
	UINT16 				incompleteSize;		// Incomplete header length
	UINT32 				frameRead;			// Remaining data to read
	HTTP_CALLBACK		callback;			// HTTP call back
	CHAR 				boundaryName[MAX_BOUNDARY_NAME_LEN];
	CHAR 				incompleteBuffer[MAX_MULTIPART_HDR_LEN + 4];
	HTTP_DATA_INFO_t	httpDataInfo;		// HTTP data info
	UINT8				audioConfigData[MAX_AUDIO_CFG_LEN];
	INT32				audioConfigDataLen;
	MP2_TS_SESSION      session;			//Mp2Ts session
}HTTP_REQUEST_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static UINT32 handleMultiPartData(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle);
//-------------------------------------------------------------------------------------------------
static UINT32 handleSinglePartData(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle);
//-------------------------------------------------------------------------------------------------
static UINT32 handleSinglePartImageData(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle);
//-------------------------------------------------------------------------------------------------
static UINT32 handleSingleResponseData(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle);
//-------------------------------------------------------------------------------------------------
static UINT32 handleMultiChunkData(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle);
//-------------------------------------------------------------------------------------------------
static UINT32 handleUnknownData(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle);
//-------------------------------------------------------------------------------------------------
static void multipartParseHdrState(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle);
//-------------------------------------------------------------------------------------------------
static void multipartGetFrameState(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle);
//-------------------------------------------------------------------------------------------------
static void sendFrameData(HTTP_HANDLE handle, BOOL appendHeader);
//-------------------------------------------------------------------------------------------------
static void findHttpStreamType(CHARPTR streamTypeStr, HTTP_REQUEST_INFO_t *reqInfoPtr);
//-------------------------------------------------------------------------------------------------
static BOOL getVideoFramePara(HTTP_DATA_INFO_t *httpDataInfoPtr);
//-------------------------------------------------------------------------------------------------
static void findSamplingFreq(CHARPTR dataPtr, HTTP_REQUEST_INFO_t *reqInfoPtr);
//-------------------------------------------------------------------------------------------------
static BOOL getByteFromStr(CHARPTR tempBuffer, UINT8PTR confByte, UINT8PTR confSize);
//-------------------------------------------------------------------------------------------------
static BOOL getNibble(CHARPTR c, UINT8PTR resNbl);
//-------------------------------------------------------------------------------------------------
static BOOL findContentLength(CHARPTR data, UINT32PTR lengthPtr);
//-------------------------------------------------------------------------------------------------
static void mp2TsFrameCallback(HTTP_HANDLE httpHandle, MP2_CLIENT_INFO_t *mp2TsData);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const CHARPTR contentLengthTag[CONTENT_LENGTH_TAG_MAX] =
{
	CONTENT_LENGTH_TAG1,
	CONTENT_LENGTH_TAG2,
};

static UINT32 (*httpDataHandler[MAX_HTTP_DATA_TYPE])(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle) =
{
	handleMultiPartData,
	handleSinglePartData,
	handleSinglePartImageData,
	handleSingleResponseData,
	handleMultiChunkData,
	handleUnknownData
};

static void (*multiPartStateHandler[MAX_MULTIPART_STATE])(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle) =
{
	multipartParseHdrState,
	multipartGetFrameState,
};

static UINT32 httpRespMappedToCmsStatusCode[MAX_HTTP_RESP_MAP_ENTRY][2] =
{
	{404,	CMD_BRND_MDL_MISMATCH},
	{401, 	CMD_CRED_INVALID},
};

static HTTP_REQUEST_INFO_t 	httpRequestInfo[MAX_HTTP_REQUEST];
static HTTP_STATE_INFO_t 	httpStateInfo[MAX_HTTP_REQUEST];

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will initialize parser for the given index.
 * @param   handle
 * @param   callback
 * @param   userData
 */
void InitHttpParser(HTTP_HANDLE handle, HTTP_CALLBACK callback, UINT32 userData)
{
    httpRequestInfo[handle].isAuthReqSent = FALSE;
    httpRequestInfo[handle].configDoneBit = 0;
    memset(httpRequestInfo[handle].boundaryName, 0, MAX_BOUNDARY_NAME_LEN);
    httpRequestInfo[handle].callback = callback;
    httpRequestInfo[handle].frameRead = 0;
    httpRequestInfo[handle].httpDataType = UNKNOWN_DATA_TYPE;
    httpRequestInfo[handle].httpDataInfo.userData = userData;
    httpRequestInfo[handle].httpDataInfo.httpResponse = MAX_HTTP_RESPONSE;
    httpRequestInfo[handle].httpDataInfo.cmdResponse = CMD_PROCESS_ERROR;
    httpRequestInfo[handle].httpDataInfo.storagePtr = NULL;
    httpRequestInfo[handle].httpDataInfo.ptrSize = 0;
    httpRequestInfo[handle].httpDataInfo.streamType = MAX_HTTP_STREAM_TYPE;
    httpRequestInfo[handle].httpDataInfo.mediaFrame.len = 0;
    httpRequestInfo[handle].httpDataInfo.mediaFrame.sampleRate = 0;
    httpRequestInfo[handle].httpDataInfo.mediaFrame.codecType = MAX_CODEC_TYPE;
    httpRequestInfo[handle].httpDataInfo.mediaFrame.videoInfo.frameType = MAX_FRAME_TYPE;
    httpRequestInfo[handle].httpDataInfo.mediaFrame.videoInfo.height = 0;
    httpRequestInfo[handle].httpDataInfo.mediaFrame.videoInfo.width = 0;
    httpRequestInfo[handle].httpDataInfo.mediaFrame.videoInfo.noOfRefFrame = 0;
    memset(httpRequestInfo[handle].incompleteBuffer, 0, MAX_MULTIPART_HDR_LEN);
    httpRequestInfo[handle].incompleteSize = 0;
    httpRequestInfo[handle].httpDataState = MULTIPART_PARSE_HEADER_STATE;
    memset(httpRequestInfo[handle].audioConfigData, 0, MAX_AUDIO_CFG_LEN);
    httpRequestInfo[handle].audioConfigDataLen = 0;
    httpRequestInfo[handle].session = MAX_MP2_SESSION;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will parse the HTTP header and extracts necessary information.
 * @param   buffer
 * @param   row
 * @param   column
 * @param   handle
 * @return
 */
UINT32 httpParseHeader(VOIDPTR buffer, UINT32 row, UINT32 column, HTTP_HANDLE handle)
{
    CHAR    tempString[100];
    UINT32  response, length;
    UINT32  datalen = (row * column);
    CHARPTR tempStr;
    CHARPTR temp;
    UINT8   loop;
    BOOL    contentLengthFound = FALSE;

    if(((tempStr = strstr((CHARPTR)buffer, HTTP_RESPONSE)) != NULL) && (httpRequestInfo[handle].httpDataInfo.httpResponse == MAX_HTTP_RESPONSE))
	{
		// then parse the response code
		sscanf((CHARPTR)buffer, "%s %d", tempString, &response);

        // if response is AUTHENTICATION FAILURE, check if the authentication process is completed or not
		if(response == HTTP_UNAUTHENTIC)
		{
			// If authentication type is not specified and it is first attempt
			// without user name and password, set flag and try with username and password
            if((httpRequestInfo[handle].isAuthReqSent == FALSE) && (GetAuthType(handle) == AUTH_TYPE_ANY))
			{
                httpRequestInfo[handle].isAuthReqSent = TRUE;
			}
            // if it is second failure attempt [with username, password OR suitable authentication method] return failure
			else
			{
                httpRequestInfo[handle].isAuthReqSent = FALSE;
			}
		}
        // if response is other then AUTHENTICATION FAILURE CLEAR authentication progress flag
		else
		{
            httpRequestInfo[handle].isAuthReqSent = FALSE;
		}

        // check the category in which the response belongs to, store it accordingly
        if(httpRequestInfo[handle].isAuthReqSent == FALSE)
		{
			httpRequestInfo[handle].httpDataInfo.storagePtr = NULL;
			httpRequestInfo[handle].httpDataInfo.frameSize = 0;

            if((response >= 200) && (response <= 299))
			{
				httpRequestInfo[handle].httpDataInfo.httpResponse = HTTP_SUCCESS;
                if((response == 204) && (httpRequestInfo[handle].callback != NULL))
				{
                    (*httpRequestInfo[handle].callback)(handle, &httpRequestInfo[handle].httpDataInfo);
				}
			}
			else
			{
				for (loop = 0; loop < MAX_HTTP_RESP_MAP_ENTRY; loop++)
				{
					if (httpRespMappedToCmsStatusCode[loop][0] == response)
					{
						httpRequestInfo[handle].httpDataInfo.cmdResponse = httpRespMappedToCmsStatusCode[loop][1];
						break;
					}
				}

				httpRequestInfo[handle].httpDataInfo.httpResponse = HTTP_ERROR;
				if(httpRequestInfo[handle].callback != NULL)
				{
                    (*httpRequestInfo[handle].callback)(handle, &httpRequestInfo[handle].httpDataInfo);
				}
				StopHttp(handle);
			}
		}
	}
	// if response is HTTP SUCCESS, then start processing the header
	else if(httpRequestInfo[handle].httpDataInfo.httpResponse == HTTP_SUCCESS)
	{
		// Search for content-type tag
		if((tempStr = strcasestr((CHARPTR)buffer, CONTENT_TYPE_TAG)) != NULL)
		{
			contentLengthFound = TRUE;
			tempStr += strlen(CONTENT_TYPE_TAG);
            while((*tempStr == ':') || (*tempStr == ' '))
            {
				tempStr++;
            }

            // Find out content type for the received stream If it contains multi-part, search for boundary name
            if(strncasecmp(tempStr, MULTIPART_TAG, strlen(MULTIPART_TAG)) == STATUS_OK)
			{
				httpRequestInfo[handle].httpDataType = MULTI_PART_DATA;
                httpRequestInfo[handle].httpDataState = MULTIPART_PARSE_HEADER_STATE;
				THREAD_START_INDEX("HTTP", handle);

				// If boundary name found save it for further parsing
                tempStr = strcasestr(tempStr, BOUNDARY_TAG);
				if(tempStr != NULL)
				{
					tempStr += strlen(BOUNDARY_TAG);
                    while((*tempStr == ':') || (*tempStr == ' ') || (*tempStr == '='))
					{
						tempStr++;
					}

					temp = strstr(tempStr, CR_LF_STR);
					if(temp == NULL)
					{
                        EPRINT(HTTP_CLIENT, "fail to extract boundary name: [handle=%d]", handle);
					}
					else
					{
						length = (temp - tempStr);
						if(length >= MAX_BOUNDARY_NAME_LEN)
						{
							length = (MAX_BOUNDARY_NAME_LEN - 1);
						}

						// NOTE:As per Mobotics camera we need to change boundary parsing metthod
						if(tempStr[0] == '"')
						{
							memcpy(tempString, (tempStr + 1), (length - 2));
							tempString[(length - 2)] = '\0';
						}
						else
						{
							memcpy(tempString, tempStr, length);
							tempString[length] = '\0';
						}

						length = 0;
                        if(memcmp(tempString, START_OF_BOUNDARY, strlen(START_OF_BOUNDARY)) != 0)
						{
                            snprintf(httpRequestInfo[handle].boundaryName, MAX_BOUNDARY_NAME_LEN, START_OF_BOUNDARY);
                            length += strlen(START_OF_BOUNDARY);
                            if(length > MAX_BOUNDARY_NAME_LEN)
                            {
                                EPRINT(HTTP_CLIENT, "length is greater than buffer: [length=%d]", length);
                                length = MAX_BOUNDARY_NAME_LEN;
                            }
						}

                        snprintf(&httpRequestInfo[handle].boundaryName[length], MAX_BOUNDARY_NAME_LEN - length, "%s", tempString);
					}
				}
			}
			// If data type is not multi-part, HTTP data can be in any other formate
			else
			{
				// Find out stream data type and sub type
				buffer = tempStr;
                if((tempStr = strchr((CHARPTR)buffer, '/')) != NULL)
				{
					length = (tempStr - (CHARPTR)buffer);
					memcpy(tempString, buffer, length);
					tempString[length] = '\0';
					findHttpStreamType(tempString, &httpRequestInfo[handle]);
					tempStr++;
					buffer = tempStr;

					if(httpRequestInfo[handle].httpDataInfo.streamType == VIDEO_STREAM)
					{
						if((tempStr = strstr((CHARPTR)buffer, CR_LF_STR)) != NULL)
						{
							length = (tempStr - (CHARPTR)buffer);
							memcpy(tempString, buffer, length);
							tempString[length] = '\0';
                            httpRequestInfo[handle].httpDataInfo.mediaFrame.codecType = GetVideoCodec(tempString);
						}
						else
						{
                            EPRINT(HTTP_CLIENT, "video codec type not specified: [handle=%d]", handle);
						}
					}
					else if(httpRequestInfo[handle].httpDataInfo.streamType == AUDIO_STREAM)
					{
                        if(((tempStr = strchr((CHARPTR)buffer, ';')) != NULL) || ((tempStr = strstr((CHARPTR)buffer, CR_LF_STR)) != NULL))
						{
							length = (tempStr - (CHARPTR)buffer);
							memcpy(tempString, buffer, length);
							tempString[length] = '\0';
                            httpRequestInfo[handle].httpDataInfo.mediaFrame.codecType = GetAudioCodec(tempString);
							findSamplingFreq(tempStr, &httpRequestInfo[handle]);
						}
						else
						{
                            EPRINT(HTTP_CLIENT, "audio codec type not specified: [handle=%d]", handle);
						}
					}
				}
			}
		}
		// Search for Transfer-Encoding tag
        else if((contentLengthFound == TRUE) && ((tempStr = strcasestr((CHARPTR)buffer, TRANSFER_ENCODE_TAG)) != NULL))
		{
			tempStr += strlen(TRANSFER_ENCODE_TAG);
            while((*tempStr == ':') || (*tempStr == ' '))
            {
				tempStr++;
            }

            // Find out content type for the received stream If it contains multi-part, it is multi-part data, now search for boundary name
			if(strncasecmp(tempStr, CHUNK_TAG, strlen(CHUNK_TAG)) == STATUS_OK)
			{
                if(StartMp2TsParser(&httpRequestInfo[handle].session) == SUCCESS)
                {
                    //Multichunk data handling
                    httpRequestInfo[handle].httpDataType = MULTI_CHUNK_DATA;
                    DPRINT(HTTP_CLIENT, "mpeg2ts parser started: session[session=%d], [handle=%d]", httpRequestInfo[handle].session, handle);
                }
			}
		}
		else
		{
            findContentLength((CHARPTR)buffer, &httpRequestInfo[handle].httpDataInfo.frameSize);
		}
	}

	return (datalen);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will pass data to the suitable function as per the response received.
 * @param   curlBuffer
 * @param   row
 * @param   column
 * @param   handle
 * @return
 */
UINT32 httpParseData(VOIDPTR curlBuffer, UINT32 row, UINT32 column, HTTP_HANDLE handle)
{
    UINT32 currPacketLen = (row * column);

    if ((curlBuffer == NULL) || (currPacketLen == 0) || (handle >= MAX_HTTP_REQUEST))
	{
        return 0;
    }

    switch(httpRequestInfo[handle].httpDataInfo.httpResponse)
    {
        case HTTP_ERROR:
        {
            httpRequestInfo[handle].httpDataInfo.storagePtr = NULL;
            httpRequestInfo[handle].httpDataInfo.frameSize = 0;
            if(httpRequestInfo[handle].callback != NULL)
            {
                (*httpRequestInfo[handle].callback)(handle, &httpRequestInfo[handle].httpDataInfo);
            }
            StopHttp(handle);
        }
        break;

        /* If HTTP response is SUCCESS, Handle data as per its type */
        case HTTP_SUCCESS:
        {
            if (httpRequestInfo[handle].httpDataType >= MAX_HTTP_DATA_TYPE)
            {
                break;
            }

            /* Allocate memory for payload data including null */
            CHARPTR payload = malloc(currPacketLen+1);
            if (NULL == payload)
            {
                EPRINT(HTTP_CLIENT, "fail to alloc memory: [handle=%d], [size=%d]", handle, currPacketLen);
                return 0;
            }

            /* Copy payload data in allocated buffer */
            memcpy(payload, curlBuffer, currPacketLen);

            /* Terminate buffer with null */
            payload[currPacketLen] = '\0';

            /* Parse response data */
            currPacketLen = (httpDataHandler[httpRequestInfo[handle].httpDataType])(payload, currPacketLen, handle);

            /* Free allocated payload memory */
            free(payload);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

	return currPacketLen;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will call registered callback function and pass the HTTP response to it.
 *          It will free the allocated memory for the given handle.
 * @param   handle
 * @param   response
 */
void CleanupHttpInfo(HTTP_HANDLE handle, HTTP_RESPONSE_e response)
{
	httpRequestInfo[handle].httpDataInfo.httpResponse = response;
    FREE_MEMORY(httpRequestInfo[handle].httpDataInfo.storagePtr);
	httpRequestInfo[handle].httpDataInfo.ptrSize = 0;
	httpRequestInfo[handle].httpDataInfo.frameSize = 0;

	if(httpRequestInfo[handle].session != MAX_MP2_SESSION)
	{
		if(StopMp2TsParser(httpRequestInfo[handle].session) == SUCCESS)
		{
			httpRequestInfo[handle].session = INVALID_MP2_SESSION;
            DPRINT(HTTP_CLIENT, "mpeg2ts parser stopped: [handle=%d]", handle);
		}
		else
		{
            EPRINT(HTTP_CLIENT, "fail to stop mpeg2ts parser: [handle=%d]", handle);
		}
	}

	if(httpRequestInfo[handle].callback != NULL)
	{
		// call user callback and send the given response to it
        (*httpRequestInfo[handle].callback)(handle, &httpRequestInfo[handle].httpDataInfo);
	}
	else
	{
        EPRINT(HTTP_CLIENT, "invld http callback: [handle=%d]", handle);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will parse multipart data and returns no of bytes parsed
 * @param   dataBuffer
 * @param   dataLen
 * @param   handle
 * @return
 */
static UINT32 handleMultiPartData(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle)
{
	httpStateInfo[handle].readPtr = dataBuffer;
	httpStateInfo[handle].parsedBytes = 0;
	httpStateInfo[handle].dataParseStatus = SUCCESS;

	do
	{
        (multiPartStateHandler[httpRequestInfo[handle].httpDataState]) (dataBuffer, dataLen, handle);
	}
	while(httpStateInfo[handle].parsedBytes < dataLen);

	if(httpStateInfo[handle].dataParseStatus == FAIL)
	{
		httpStateInfo[handle].parsedBytes = 0;
	}

	return(httpStateInfo[handle].parsedBytes);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will collect single part data and pass it to the respective function
 * @param   dataBuffer
 * @param   dataLen
 * @param   handle
 * @return
 */
static UINT32 handleSinglePartData(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle)
{
    VOIDPTR tmpPtr = NULL;

	httpStateInfo[handle].readPtr = dataBuffer;
	httpStateInfo[handle].parsedBytes = 0;
	httpStateInfo[handle].dataParseStatus = SUCCESS;

	if(httpRequestInfo[handle].httpDataInfo.storagePtr == NULL)
	{
        httpRequestInfo[handle].httpDataInfo.storagePtr = malloc(dataLen+1);
	}
	else if(httpRequestInfo[handle].httpDataInfo.ptrSize < dataLen)
	{
        tmpPtr = realloc(httpRequestInfo[handle].httpDataInfo.storagePtr, dataLen+1);
		if(tmpPtr == NULL)
		{
			free(httpRequestInfo[handle].httpDataInfo.storagePtr);
			httpRequestInfo[handle].httpDataInfo.storagePtr = NULL;
		}
		else
		{
			httpRequestInfo[handle].httpDataInfo.storagePtr = tmpPtr;
		}
	}

	if(httpRequestInfo[handle].httpDataInfo.storagePtr == NULL)
	{
        EPRINT(HTTP_CLIENT, "fail to allocate memory: [handle=%d]", handle);
		httpStateInfo[handle].dataParseStatus = FAIL;
		httpRequestInfo[handle].httpDataInfo.ptrSize = 0;
		httpStateInfo[handle].parsedBytes = dataLen;
	}
	else
	{
        memcpy(httpRequestInfo[handle].httpDataInfo.storagePtr, dataBuffer, dataLen);
		httpRequestInfo[handle].httpDataInfo.ptrSize = dataLen;
		httpRequestInfo[handle].httpDataInfo.frameSize = dataLen;
		httpStateInfo[handle].parsedBytes = dataLen;
        ((CHARPTR)httpRequestInfo[handle].httpDataInfo.storagePtr)[dataLen] = '\0';
		sendFrameData(handle, TRUE);
	}

	return(httpStateInfo[handle].parsedBytes);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will collect single part data and pass it to the respective function
 * @param   dataBuffer
 * @param   dataLen
 * @param   handle
 * @return
 */
static UINT32 handleSinglePartImageData(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle)
{
	BOOL		status = FAIL;
	UINT32		dataByte;
	CHARPTR		tempPtr = NULL;

	if(httpRequestInfo[handle].httpDataInfo.storagePtr == NULL)
	{
		for(dataByte = 0; dataByte < dataLen; dataByte++)
		{
			if((UINT8)dataBuffer[dataByte] == 0xff)
			{
				if((UINT8)dataBuffer[dataByte + 1] == 0xd8)
				{
					status = SUCCESS;
					break;
				}
			}
		}

		if(status == SUCCESS)
		{
			if(httpRequestInfo[handle].httpDataInfo.storagePtr == NULL)
			{
				httpRequestInfo[handle].httpDataInfo.storagePtr = malloc(dataLen);
			}
			else if(httpRequestInfo[handle].httpDataInfo.ptrSize < dataLen)
			{
                tempPtr = realloc(httpRequestInfo[handle].httpDataInfo.storagePtr, dataLen);
				if(tempPtr == NULL)
				{
					free(httpRequestInfo[handle].httpDataInfo.storagePtr);
					httpRequestInfo[handle].httpDataInfo.storagePtr = NULL;
				}
				else
				{
					httpRequestInfo[handle].httpDataInfo.storagePtr = tempPtr;					
				}
			}

			if(httpRequestInfo[handle].httpDataInfo.storagePtr == NULL)
			{
				// if failed to allocate memory, make read packet bytes zero
                EPRINT(HTTP_CLIENT, "fail to alloc memory: [handle=%d]", handle);
				httpStateInfo[handle].dataParseStatus = FAIL;
				httpStateInfo[handle].parsedBytes = dataLen;
				httpRequestInfo[handle].httpDataInfo.ptrSize = 0;
			}
			else
			{
                memcpy(httpRequestInfo[handle].httpDataInfo.storagePtr, dataBuffer, dataLen);
				httpRequestInfo[handle].httpDataInfo.frameSize = dataLen;
				httpStateInfo[handle].parsedBytes = dataLen;
				httpRequestInfo[handle].httpDataInfo.ptrSize = dataLen;

				for(dataByte = (dataLen - 1); dataByte >= 2; dataByte--)
				{
					if((UINT8)dataBuffer[dataByte] == 0xd9)
					{
						if((UINT8)dataBuffer[dataByte - 1] == 0xff)
						{
                            DPRINT(HTTP_CLIENT, "complete mjpg frame received: [handle=%d]", handle);
							sendFrameData(handle, TRUE);
						}
					}
				}
			}
		}
		else
		{
			// if failed get valid jpeg start code
            EPRINT(HTTP_CLIENT, "jpeg start code not found: [handle=%d]", handle);
			httpStateInfo[handle].dataParseStatus = FAIL;
			httpStateInfo[handle].parsedBytes = dataLen;
		}
	}
	else
	{
		for(dataByte = (dataLen - 1); dataByte >= 1; dataByte--)
		{
			if((UINT8)dataBuffer[dataByte] == 0xd9)
			{
				if((UINT8)dataBuffer[dataByte - 1] == 0xff)
				{
					status = SUCCESS;
				}
			}
		}

        if(httpRequestInfo[handle].httpDataInfo.ptrSize < (httpRequestInfo[handle].httpDataInfo.frameSize + dataLen))
		{
            tempPtr = realloc(httpRequestInfo[handle].httpDataInfo.storagePtr, (httpRequestInfo[handle].httpDataInfo.frameSize + dataLen));
            httpRequestInfo[handle].httpDataInfo.ptrSize = (httpRequestInfo[handle].httpDataInfo.frameSize + dataLen);
		}

		if(tempPtr == NULL)
		{
			// if failed to allocate memory, make read packet bytes zero
            EPRINT(HTTP_CLIENT, "fail to realloc memory: [handle=%d]", handle);
			free(httpRequestInfo[handle].httpDataInfo.storagePtr);
			httpRequestInfo[handle].httpDataInfo.storagePtr = NULL;
			httpRequestInfo[handle].httpDataInfo.ptrSize = 0;
			httpRequestInfo[handle].httpDataInfo.frameSize = 0;
			httpStateInfo[handle].dataParseStatus = FAIL;
			httpStateInfo[handle].parsedBytes = dataLen;
		}
		else
		{
			httpRequestInfo[handle].httpDataInfo.storagePtr = tempPtr;
            memcpy((httpRequestInfo[handle].httpDataInfo.storagePtr + httpRequestInfo[handle].httpDataInfo.frameSize), dataBuffer, dataLen);
			httpRequestInfo[handle].httpDataInfo.frameSize += dataLen;
			httpStateInfo[handle].parsedBytes = dataLen;
			if(status == SUCCESS)
			{
                DPRINT(HTTP_CLIENT, "complete mjpg frame received: [handle=%d]", handle);
				sendFrameData(handle, TRUE);
			}
		}
	}

	return(httpStateInfo[handle].parsedBytes);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will collect single response data and pass it to the respective function
 *          once whole frame is received.
 * @param   dataBuffer
 * @param   dataLen
 * @param   handle
 * @return
 */
static UINT32 handleSingleResponseData(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle)
{
	UINT32	length;
	VOIDPTR	tempPtr = NULL;

	// Initialise state variables
    httpStateInfo[handle].readPtr = dataBuffer;
    httpStateInfo[handle].parsedBytes = 0;
    httpStateInfo[handle].dataParseStatus = SUCCESS;

	if(httpRequestInfo[handle].httpDataInfo.storagePtr == NULL)
	{
        httpRequestInfo[handle].httpDataInfo.storagePtr = malloc(httpRequestInfo[handle].httpDataInfo.frameSize+1);
		if(httpRequestInfo[handle].httpDataInfo.storagePtr == NULL)
		{
			// if failed to allocate memory, make read packet bytes zero
            EPRINT(HTTP_CLIENT, "fail to alloc memory: [handle=%d]", handle);
			httpStateInfo[handle].dataParseStatus = FAIL;
			httpStateInfo[handle].parsedBytes = dataLen;
			httpRequestInfo[handle].httpDataInfo.ptrSize = 0;
		}
		else
		{
			httpRequestInfo[handle].frameRead = 0;
		}
	}
    else if (httpRequestInfo[handle].httpDataInfo.ptrSize < httpRequestInfo[handle].httpDataInfo.frameSize)
	{
        tempPtr = realloc(httpRequestInfo[handle].httpDataInfo.storagePtr, httpRequestInfo[handle].httpDataInfo.frameSize+1);
		if(tempPtr == NULL)
		{
			// if failed to allocate memory, make read packet bytes zero
            EPRINT(HTTP_CLIENT, "fail to realloc memory: [handle=%d]", handle);
			free(httpRequestInfo[handle].httpDataInfo.storagePtr);
			httpRequestInfo[handle].httpDataInfo.storagePtr = NULL;
			httpStateInfo[handle].dataParseStatus = FAIL;
			httpStateInfo[handle].parsedBytes = dataLen;
			httpRequestInfo[handle].httpDataInfo.ptrSize = 0;
		}
		else
		{
			httpRequestInfo[handle].httpDataInfo.storagePtr = tempPtr;
		}
	}

	if(httpRequestInfo[handle].httpDataInfo.storagePtr != NULL)
	{
		// If still more data to come, save received data
        if((httpRequestInfo[handle].httpDataInfo.frameSize - httpRequestInfo[handle].frameRead) > dataLen)
		{
            // if remaining frame size is greater then or equal to reminaning packet size copy remaining packet to storage buffer
            memcpy((httpRequestInfo[handle].httpDataInfo.storagePtr + httpRequestInfo[handle].frameRead), httpStateInfo[handle].readPtr, dataLen);

			// update the frame read bytes
			httpRequestInfo[handle].frameRead += dataLen;

			// update read pointer of current packet
			httpStateInfo[handle].readPtr += dataLen;

			// update count of bytes actually read from current packet
			httpStateInfo[handle].parsedBytes = dataLen;
		}
		// If data received, Change state and pass it on
		else
		{
            length = (httpRequestInfo[handle].httpDataInfo.frameSize - httpRequestInfo[handle].frameRead);

            // if remaining frame is lesser then remaining packet size, copy remaining frame size data from current packet to storage buffer
            memcpy((httpRequestInfo[handle].httpDataInfo.storagePtr + httpRequestInfo[handle].frameRead), httpStateInfo[handle].readPtr, length);

			// update read pointer of current packet for next read
			httpStateInfo[handle].readPtr += length;

			// update count of bytes actually read from current packet
			httpStateInfo[handle].parsedBytes = dataLen;
            ((CHARPTR)httpRequestInfo[handle].httpDataInfo.storagePtr)[httpRequestInfo[handle].httpDataInfo.frameSize] = '\0';
			sendFrameData(handle, TRUE);
		}
	}

	return(httpStateInfo[handle].parsedBytes);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will send chunk data to parser. After parsing data it send to CI.
 * @param   dataBuffer
 * @param   dataLen
 * @param   handle
 * @return
 */
static UINT32 handleMultiChunkData(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle)
{
    MP2_CLIENT_INFO_t mp2ClientInfo;

	// Send Frame to parse Session
	mp2ClientInfo.session = httpRequestInfo[handle].session;
	if(mp2ClientInfo.session >= MAX_MP2_SESSION)
	{
        EPRINT(HTTP_CLIENT, "invld mpeg2ts session: [handle=%d]", handle);
		return dataLen;
	}

	dataBuffer[dataLen] = '\0';
    mp2ClientInfo.data = (UINT8PTR)dataBuffer;
	mp2ClientInfo.dataLen = dataLen;

	do
	{
		//Initialize mp2DataInfo
		mp2ClientInfo.mp2DataInfo.streamType = MAX_STREAM_TYPE;
		mp2ClientInfo.mp2DataInfo.codecType  = MAX_CODEC_TYPE;
		mp2ClientInfo.mp2DataInfo.frameSize  = 0;

		if(ParseMp2TsData(handle, &mp2ClientInfo, mp2TsFrameCallback) == FAIL)
		{
            //Stop stream
            EPRINT(HTTP_CLIENT, "mpeg2ts data parse failed: [handle=%d]", handle);
			StopHttp(handle);
			break;
		}
	}
	while(mp2ClientInfo.mp2DataInfo.frameSize != 0);//Loop till frames are available

	return dataLen;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   mp2TsFrameCallback
 * @param   httpHandle
 * @param   mp2TsData
 */
static void mp2TsFrameCallback(HTTP_HANDLE httpHandle, MP2_CLIENT_INFO_t *mp2TsData)
{
	//Check the result whether frame available or not
    if (mp2TsData->mp2DataInfo.streamType != MAX_STREAM_TYPE)
	{
        httpRequestInfo[httpHandle].httpDataInfo.storagePtr = mp2TsData->mp2DataInfo.framePtr;
        httpRequestInfo[httpHandle].httpDataInfo.frameSize = mp2TsData->mp2DataInfo.frameSize;
        httpRequestInfo[httpHandle].httpDataInfo.mediaFrame.codecType = mp2TsData->mp2DataInfo.codecType;
		if(mp2TsData->mp2DataInfo.streamType == STREAM_TYPE_VIDEO)
		{
			httpRequestInfo[httpHandle].httpDataInfo.streamType = VIDEO_STREAM;
		}
		else
		{
			httpRequestInfo[httpHandle].httpDataInfo.streamType = AUDIO_STREAM;

			//If Audio Type is AAC then	extract its sampling frequency
            if(httpRequestInfo[httpHandle].httpDataInfo.mediaFrame.codecType == AUDIO_AAC)
			{
                httpRequestInfo[httpHandle].httpDataInfo.mediaFrame.sampleRate =
                        GetAACSamplingFreq((UINT8PTR)(httpRequestInfo[httpHandle].httpDataInfo.storagePtr));
			}
		}

		sendFrameData(httpHandle, FALSE);
		httpRequestInfo[httpHandle].httpDataInfo.storagePtr = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will parse the received data and identify whether the data will be
 *          received in multipart or singlepart.
 * @param   dataBuffer
 * @param   dataLen
 * @param   handle
 * @return
 */
static UINT32 handleUnknownData(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle)
{
	UINT32		length;
	CHAR		tempString[100];
	CHARPTR 	crlfPtr;
	CHARPTR		startBoundPtr;

	if(httpRequestInfo[handle].httpDataInfo.frameSize > 0)
	{
		httpRequestInfo[handle].httpDataType = SINGLE_RESPONSE_DATA;
	}
	else
	{
        length = (dataLen > MAX_MULTIPART_HDR_LEN) ? MAX_MULTIPART_HDR_LEN : dataLen;
		memcpy(httpRequestInfo[handle].incompleteBuffer, dataBuffer, length);
		httpRequestInfo[handle].incompleteBuffer[length] = '\0';
		httpRequestInfo[handle].incompleteSize = length;
		httpRequestInfo[handle].httpDataType = SINGLE_PART_DATA;

        if((startBoundPtr = strstr(httpRequestInfo[handle].incompleteBuffer, START_OF_BOUNDARY)) != NULL)
		{
            if((crlfPtr = strstr(startBoundPtr, CR_LF_STR)) != NULL)
			{
				length = (crlfPtr - startBoundPtr);
				if(length >= MAX_BOUNDARY_NAME_LEN)
				{
					length = (MAX_BOUNDARY_NAME_LEN - 1);
				}

				memcpy(tempString, startBoundPtr, length);
				tempString[length] = '\0';
				length = 0;

                if(memcmp(tempString, START_OF_BOUNDARY, strlen(START_OF_BOUNDARY)) != 0)
				{
                    snprintf(httpRequestInfo[handle].boundaryName, MAX_BOUNDARY_NAME_LEN, START_OF_BOUNDARY);
					length += strlen(START_OF_BOUNDARY);
                    if(length > MAX_BOUNDARY_NAME_LEN)
                    {
                        EPRINT(HTTP_CLIENT, "length is greater than buffer: [length=%d]", length);
                        length = MAX_BOUNDARY_NAME_LEN;
                    }
				}

                snprintf(&httpRequestInfo[handle].boundaryName[length], MAX_BOUNDARY_NAME_LEN - length, "%s", tempString);
				httpRequestInfo[handle].httpDataType = MULTI_PART_DATA;
				httpRequestInfo[handle].httpDataState = MULTIPART_PARSE_HEADER_STATE;
				THREAD_START_INDEX("HTTP", handle);
			}
		}
		else
		{
			//If http stream type not available then try it for mpeg2ts parser
			if(httpRequestInfo[handle].httpDataInfo.streamType == MAX_HTTP_STREAM_TYPE)
			{
				if(StartMp2TsParser(&httpRequestInfo[handle].session) == SUCCESS)
				{
					httpRequestInfo[handle].httpDataType = MULTI_CHUNK_DATA;
                    DPRINT(HTTP_CLIENT, "mpeg2ts parser started: [session=%d], [handle=%d]", httpRequestInfo[handle].session, handle);
				}
			}
		}

		if(httpRequestInfo[handle].httpDataType == SINGLE_PART_DATA)
		{
            if((httpRequestInfo[handle].httpDataInfo.streamType == VIDEO_STREAM)
                    && (httpRequestInfo[handle].httpDataInfo.mediaFrame.codecType == VIDEO_MJPG))
			{
				httpRequestInfo[handle].httpDataType = SINGLE_PART_IMAGE_DATA;
                DPRINT(HTTP_CLIENT, "video mjpg data received in single part: [handle=%d]", handle);
			}
		}
	}

	httpRequestInfo[handle].incompleteSize = 0;
    return (httpDataHandler[httpRequestInfo[handle].httpDataType])(dataBuffer, dataLen, handle);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will parse multipart header. It extracts data information like stream type,
 *          codec type, size etc from the header. bytes parsed
 * @param   dataBuffer
 * @param   dataLen
 * @param   handle
 */
static void multipartParseHdrState(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle)
{
	BOOL		procF = FALSE, boundaryFound = FALSE;
	UINT32		length, parseHdrLen = 0, lengthBuf = 0;
	CHARPTR		parseHdrPtr = NULL;
	CHARPTR		tempPtr = NULL;
	CHARPTR		hdrFieldStart;
	CHARPTR		tempHdr;
	CHAR		hdrLine[256];
	const UINT8 tmpStrLen = 128;
	CHAR		tempString[tmpStrLen];

	if(httpRequestInfo[handle].incompleteSize == 0)
	{
		hdrFieldStart = httpStateInfo[handle].readPtr;
	}
	else
	{
		lengthBuf = (MAX_MULTIPART_HDR_LEN - httpRequestInfo[handle].incompleteSize);
		if(dataLen < lengthBuf)
		{
			lengthBuf = dataLen;
		}

        memcpy(&httpRequestInfo[handle].incompleteBuffer [httpRequestInfo[handle].incompleteSize], httpStateInfo[handle].readPtr, lengthBuf);
        httpRequestInfo[handle].incompleteBuffer[httpRequestInfo[handle].incompleteSize + lengthBuf] = '\0';
		hdrFieldStart = httpRequestInfo[handle].incompleteBuffer;
	}

	// If CR_LF_STR found, it is end of a line in header, parse it
    while((parseHdrPtr = strstr(hdrFieldStart, CR_LF_STR)) != NULL)
	{
		parseHdrPtr += strlen(CR_LF_STR);
		parseHdrLen += (parseHdrPtr - hdrFieldStart);
		length = (parseHdrPtr - hdrFieldStart);

        /* We should not parse/read more than data buffer size */
        if ((httpStateInfo[handle].parsedBytes + (parseHdrLen - strlen(CR_LF_STR))) > dataLen)
        {
            WPRINT(HTTP_CLIENT, "invld curl buffer size: [handle=%d], [dataLen=%d], [parseHdrLen=%d], [parsedBytes=%d]",
                   handle, dataLen, parseHdrLen, httpStateInfo[handle].parsedBytes);
            break;
        }

		if(length >= sizeof(hdrLine))
		{
            EPRINT(HTTP_CLIENT, "header line buffer is small: [handle=%d], [length=%d]", handle, length);
			hdrFieldStart = parseHdrPtr;
			continue;
		}

		memcpy(hdrLine, hdrFieldStart, length);
		hdrLine[length] = '\0';
		hdrFieldStart = parseHdrPtr;

		// Check if the line contains boundary name
        if((tempPtr = strstr(hdrLine, httpRequestInfo[handle].boundaryName)) != NULL)
		{
			boundaryFound = SUCCESS;
		}
		// Check if the line contains content type
        else if((tempPtr = strcasestr(hdrLine, CONTENT_TYPE_TAG)) != NULL)
		{
			tempPtr += strlen(CONTENT_TYPE_TAG);
            while((*tempPtr == ':') || (*tempPtr == ' '))
            {
				tempPtr++;
            }

			tempHdr = tempPtr;
            if((tempPtr = strchr(tempHdr, '/')) != NULL)
			{
                /* here the size is kept +1 to fill up the null char at the end actual string*/
                length = (tempPtr - tempHdr) + 1;
                snprintf(tempString, length > tmpStrLen ? tmpStrLen : length, "%s", tempHdr);
				findHttpStreamType(tempString, &httpRequestInfo[handle]);
				tempHdr = (tempPtr + 1);
                if((tempPtr = strstr(tempHdr, CR_LF_STR)) != NULL)
				{
                    /* here the size is kept +1 to fill up the null char at the end actual string*/
                    length = (tempPtr - tempHdr) + 1;
                    snprintf(tempString, length > tmpStrLen ? tmpStrLen : length, "%s", tempHdr);
                    if (httpRequestInfo[handle].httpDataInfo.streamType == VIDEO_STREAM)
					{
                        httpRequestInfo[handle].httpDataInfo.mediaFrame.codecType = GetVideoCodec(tempString);

					}
                    else if(httpRequestInfo[handle].httpDataInfo.streamType == AUDIO_STREAM)
					{
                        httpRequestInfo[handle].httpDataInfo.mediaFrame.codecType = GetAudioCodec(tempString);
						findSamplingFreq(tempPtr, &httpRequestInfo[handle]);
					}
				}
			}
			else
			{
                EPRINT(HTTP_CLIENT, "http client content sub type not found: [handle=%d]", handle);
				httpStateInfo[handle].parsedBytes = dataLen;
				httpStateInfo[handle].dataParseStatus = FAIL;
				return;
			}
		}
		// Find out content length
        else if(findContentLength(hdrLine, &httpRequestInfo[handle].httpDataInfo.frameSize) == SUCCESS)
		{
			// Length found
		}
        else if(strncmp(hdrLine, CR_LF_STR, strlen(CR_LF_STR)) == 0)
		{
			if(boundaryFound == SUCCESS)
			{
				procF = TRUE;
				break;
			}
		}
	}

	if(procF == TRUE)
	{
        httpStateInfo[handle].parsedBytes += (parseHdrLen - httpRequestInfo[handle].incompleteSize);
        httpStateInfo[handle].readPtr = (dataBuffer + httpStateInfo[handle].parsedBytes);
		httpRequestInfo[handle].incompleteSize = 0;
		httpRequestInfo[handle].httpDataState = MULTIPART_GET_FRAME_STATE;
        return;
	}

    if(lengthBuf == 0)
    {
        if(dataLen > httpStateInfo[handle].parsedBytes)
        {
            lengthBuf = (dataLen - httpStateInfo[handle].parsedBytes);
            if((httpRequestInfo[handle].incompleteSize + lengthBuf) <= MAX_MULTIPART_HDR_LEN)
            {
                /* here the size is kept +1 to fill up the null char at the end actual string*/
                snprintf(&httpRequestInfo[handle].incompleteBuffer[httpRequestInfo[handle].incompleteSize],
                        lengthBuf+1, "%s", httpStateInfo[handle].readPtr);
                httpRequestInfo[handle].incompleteSize += lengthBuf;
            }
            else
            {
                // No space to store header in buffer
                EPRINT(HTTP_CLIENT, "http client buff overflow: [handle=%d], [length=%d]", handle, lengthBuf);
                httpStateInfo[handle].dataParseStatus = FAIL;
            }
        }
        else
        {
            EPRINT(HTTP_CLIENT, "http client more data: [handle=%d], [length=%d]", handle, lengthBuf);
            httpStateInfo[handle].dataParseStatus = FAIL;
        }
    }
    else
    {
        httpRequestInfo[handle].incompleteSize += lengthBuf;
    }
    httpStateInfo[handle].parsedBytes = dataLen;
    httpStateInfo[handle].readPtr = NULL;

}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will collect multipart data from the packet received. once a complete
 *          frame is received It will pass it to the respective function.
 * @param   dataBuffer
 * @param   dataLen
 * @param   handle
 */
static void multipartGetFrameState(CHARPTR dataBuffer, UINT32 dataLen, HTTP_HANDLE handle)
{
	BOOL		bNameFound = FALSE;
	UINT32		currFrmDataLen = (dataLen - httpStateInfo[handle].parsedBytes);
	CHARPTR 	tempPtr = NULL;
	VOIDPTR		newDataPtr = NULL;

	// If frame size is specified in the header, collect data for that size
	if(httpRequestInfo[handle].httpDataInfo.frameSize > 0)
	{
		// If it is start of the frame, allocate buffer
		if(httpRequestInfo[handle].httpDataInfo.storagePtr == NULL)
		{
            httpRequestInfo[handle].httpDataInfo.storagePtr = malloc(httpRequestInfo[handle].httpDataInfo.frameSize+1);
			if(httpRequestInfo[handle].httpDataInfo.storagePtr == NULL)
			{
                EPRINT(HTTP_CLIENT, "fail to alloc memory: [handle=%d]", handle);
				httpStateInfo[handle].dataParseStatus = FAIL;
				httpStateInfo[handle].parsedBytes = dataLen;
				httpRequestInfo[handle].httpDataInfo.ptrSize = 0;
				return;
			}

            httpRequestInfo[handle].frameRead = 0;
            httpStateInfo[handle].dataProcF = FALSE;
            httpRequestInfo[handle].httpDataInfo.ptrSize = httpRequestInfo[handle].httpDataInfo.frameSize;
		}
        else if (httpRequestInfo[handle].httpDataInfo.ptrSize < httpRequestInfo[handle].httpDataInfo.frameSize)
		{
            newDataPtr = realloc(httpRequestInfo[handle].httpDataInfo.storagePtr, httpRequestInfo[handle].httpDataInfo.frameSize+1);
			if(newDataPtr == NULL)
			{
                EPRINT(HTTP_CLIENT, "fail to realloc memory: [handle=%d]", handle);
				httpStateInfo[handle].dataParseStatus = FAIL;
				free(httpRequestInfo[handle].httpDataInfo.storagePtr);
				httpRequestInfo[handle].httpDataInfo.storagePtr = NULL;
				httpRequestInfo[handle].httpDataInfo.ptrSize = 0;
				httpStateInfo[handle].parsedBytes = dataLen;
				return;
			}

            httpRequestInfo[handle].frameRead = 0;
            httpStateInfo[handle].dataProcF = FALSE;
            httpRequestInfo[handle].httpDataInfo.ptrSize = httpRequestInfo[handle].httpDataInfo.frameSize;
            httpRequestInfo[handle].httpDataInfo.storagePtr = newDataPtr;
		}
        else if (httpRequestInfo[handle].httpDataInfo.ptrSize > httpRequestInfo[handle].httpDataInfo.frameSize)
		{
            memset((httpRequestInfo[handle].httpDataInfo.storagePtr + httpRequestInfo[handle].httpDataInfo.frameSize), '\0',
					(httpRequestInfo[handle].httpDataInfo.ptrSize - httpRequestInfo[handle].httpDataInfo.frameSize));
		}

		if(httpStateInfo[handle].dataProcF == FALSE)
		{
			while(currFrmDataLen > 0)
			{
                if((*httpStateInfo[handle].readPtr == '\r') || (*httpStateInfo[handle].readPtr == '\n'))
				{
					httpStateInfo[handle].readPtr++;
					httpStateInfo[handle].parsedBytes++;
					currFrmDataLen--;
				}
				else
				{
					httpStateInfo[handle].dataProcF = TRUE;
					currFrmDataLen = (dataLen - httpStateInfo[handle].parsedBytes);
					break;
				}
			}
		}

		// If still more data to come, save received data
        if((httpRequestInfo[handle].httpDataInfo.frameSize - httpRequestInfo[handle].frameRead) <= currFrmDataLen)
		{
            currFrmDataLen = (httpRequestInfo[handle].httpDataInfo.frameSize - httpRequestInfo[handle].frameRead);
			httpRequestInfo[handle].httpDataState = MULTIPART_PARSE_HEADER_STATE;
		}

        // if remaining frame size is greater then or equal to reminaning packet size copy remaining packet to storage buffer
        if (currFrmDataLen > 0)
        {
            memcpy((httpRequestInfo[handle].httpDataInfo.storagePtr + httpRequestInfo[handle].frameRead),
                   httpStateInfo[handle].readPtr, currFrmDataLen);
        }

		// update count of bytes aMULTIPART_PARSE_HEADER_STATEctually read from current packet
		httpStateInfo[handle].parsedBytes += currFrmDataLen;

		// update the frame read bytes
		httpRequestInfo[handle].frameRead += currFrmDataLen;

		// update read pointer of current packet
		httpStateInfo[handle].readPtr += currFrmDataLen;
		if(httpRequestInfo[handle].httpDataState == MULTIPART_PARSE_HEADER_STATE)
		{
            ((CHARPTR)httpRequestInfo[handle].httpDataInfo.storagePtr)[httpRequestInfo[handle].httpDataInfo.frameSize] = '\0';
			sendFrameData(handle, TRUE);
		}
	}
	// If frame size is not specified, data will be collected within two boundaries
	else
	{
        tempPtr = memmem(httpStateInfo[handle].readPtr, currFrmDataLen, httpRequestInfo[handle].boundaryName, strlen(httpRequestInfo[handle].boundaryName));
		if(tempPtr != NULL)
		{
			bNameFound = TRUE;
			currFrmDataLen = (UINT32)(tempPtr - httpStateInfo[handle].readPtr);
		}

		if(httpRequestInfo[handle].httpDataInfo.storagePtr == NULL)
		{
            httpRequestInfo[handle].httpDataInfo.storagePtr = malloc(currFrmDataLen+1);
			httpRequestInfo[handle].httpDataInfo.ptrSize = currFrmDataLen;
		}
        else if(httpRequestInfo[handle].httpDataInfo.ptrSize < (httpRequestInfo[handle].frameRead + currFrmDataLen))
		{
            newDataPtr = realloc(httpRequestInfo[handle].httpDataInfo.storagePtr, (httpRequestInfo[handle].frameRead + currFrmDataLen)+1);
			if(newDataPtr != NULL)
			{
				httpRequestInfo[handle].httpDataInfo.storagePtr = newDataPtr;
                httpRequestInfo[handle].httpDataInfo.ptrSize = (httpRequestInfo[handle].frameRead + currFrmDataLen);
			}
			else
			{
				free(httpRequestInfo[handle].httpDataInfo.storagePtr);
				httpRequestInfo[handle].httpDataInfo.storagePtr = NULL;
				httpRequestInfo[handle].httpDataInfo.ptrSize = 0;
			}
		}

		if(httpRequestInfo[handle].httpDataInfo.storagePtr != NULL)
		{
            memcpy((httpRequestInfo[handle].httpDataInfo.storagePtr + httpRequestInfo[handle].frameRead), httpStateInfo[handle].readPtr, currFrmDataLen);
			httpRequestInfo[handle].frameRead += currFrmDataLen;
			if(bNameFound == TRUE)
			{
				httpStateInfo[handle].parsedBytes += currFrmDataLen;
				httpStateInfo[handle].readPtr += currFrmDataLen;
                httpRequestInfo[handle].httpDataInfo.frameSize = httpRequestInfo[handle].frameRead;
                ((CHARPTR)httpRequestInfo[handle].httpDataInfo.storagePtr)[httpRequestInfo[handle].httpDataInfo.frameSize] = '\0';
				sendFrameData(handle, TRUE);
				httpRequestInfo[handle].httpDataState = MULTIPART_PARSE_HEADER_STATE;
			}
			else
			{
				httpStateInfo[handle].parsedBytes = dataLen;
				httpStateInfo[handle].readPtr = NULL;
			}
		}
		else
		{
			httpRequestInfo[handle].httpDataInfo.frameSize = 0;
			httpRequestInfo[handle].frameRead = 0;
			httpStateInfo[handle].parsedBytes = dataLen;
			httpStateInfo[handle].dataParseStatus = FAIL;
            EPRINT(HTTP_CLIENT, "fail to alloc memory: [handle=%d]", handle);
		}
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will identify the frame type if its video data, append header if audio
 *          data and then pass it to the respective function.
 * @param   handle
 * @param   appendHeader
 */
static void sendFrameData(HTTP_HANDLE handle, BOOL appendHeader)
{
    BOOL    status = SUCCESS;
    VOIDPTR tempDataPtr = NULL;
    VOIDPTR storgDataPtr;

	httpRequestInfo[handle].httpDataInfo.mediaFrame.isRTSP = FALSE;
	switch(httpRequestInfo[handle].httpDataInfo.streamType)
	{
        // If TEXT data is received, call respective callback
        default:
        case TEXT_STREAM:
        {
            /* Nothing to do */
        }
        break;

        // If received data is video, find video frame type n send data to the callback
        case VIDEO_STREAM:
        {
            status = getVideoFramePara(&httpRequestInfo[handle].httpDataInfo);
        }
        break;

        case AUDIO_STREAM:
        {
            if(httpRequestInfo[handle].httpDataInfo.mediaFrame.codecType == AUDIO_AAC)
            {
                // Remove first 4 bytes which is header needs to be removed
                if(appendHeader != TRUE)
                {
                    break;
                }

                httpRequestInfo[handle].httpDataInfo.frameSize -= AAC_AUDIO_HDR_LEN;
                tempDataPtr = malloc(httpRequestInfo[handle].httpDataInfo.frameSize + httpRequestInfo[handle].audioConfigDataLen);
                if(tempDataPtr == NULL)
                {
                    EPRINT(HTTP_CLIENT, "fail to alloc memory: [handle=%d]", handle);
                    status = FAIL;
                }
                else
                {
                    memcpy(tempDataPtr, httpRequestInfo[handle].audioConfigData, httpRequestInfo[handle].audioConfigDataLen);
                    memcpy((tempDataPtr + httpRequestInfo[handle].audioConfigDataLen),
                           (httpRequestInfo[handle].httpDataInfo.storagePtr + AAC_AUDIO_HDR_LEN), httpRequestInfo[handle].httpDataInfo.frameSize);
                    httpRequestInfo[handle].httpDataInfo.frameSize += httpRequestInfo[handle].audioConfigDataLen;
                    if(httpRequestInfo[handle].httpDataInfo.ptrSize < httpRequestInfo[handle].httpDataInfo.frameSize)
                    {
                        storgDataPtr = realloc(httpRequestInfo[handle].httpDataInfo.storagePtr, httpRequestInfo[handle].httpDataInfo.frameSize);
                        if(storgDataPtr == NULL)
                        {
                            EPRINT(HTTP_CLIENT, "fail to realloc memory: [handle=%d]", handle);
                            status = FAIL;
                        }
                        else
                        {
                            httpRequestInfo[handle].httpDataInfo.storagePtr = storgDataPtr;
                            httpRequestInfo[handle].httpDataInfo.ptrSize = httpRequestInfo[handle].httpDataInfo.frameSize;
                        }
                    }

                    if(status == SUCCESS)
                    {
                        memcpy(httpRequestInfo[handle].httpDataInfo.storagePtr, tempDataPtr, httpRequestInfo[handle].httpDataInfo.frameSize);
                    }
                    free(tempDataPtr);
                }

                ApendAACFrameLen((UINT8PTR)httpRequestInfo[handle].httpDataInfo.storagePtr, httpRequestInfo[handle].httpDataInfo.frameSize);
            }
            else if ((httpRequestInfo[handle].httpDataInfo.mediaFrame.codecType == AUDIO_PCM_L)
                     || (httpRequestInfo[handle].httpDataInfo.mediaFrame.codecType == AUDIO_PCM_B))
            {
                if(httpRequestInfo[handle].httpDataInfo.frameSize > PCM_AUDIO_HDR_LEN)
                {
                    // Remove first 4 bytes which is header needs to be removed
                    httpRequestInfo[handle].httpDataInfo.frameSize -= PCM_AUDIO_HDR_LEN;
                    tempDataPtr = malloc(httpRequestInfo[handle].httpDataInfo.frameSize);
                    if(tempDataPtr == NULL)
                    {
                        EPRINT(HTTP_CLIENT, "fail to alloc memory: [handle=%d]", handle);
                        status = FAIL;
                    }
                    else
                    {
                        memcpy(tempDataPtr, (httpRequestInfo[handle].httpDataInfo.storagePtr + PCM_AUDIO_HDR_LEN), httpRequestInfo[handle].httpDataInfo.frameSize);
                        memcpy(httpRequestInfo[handle].httpDataInfo.storagePtr, tempDataPtr, httpRequestInfo[handle].httpDataInfo.frameSize);
                        free(tempDataPtr);
                    }
                }
            }

            httpRequestInfo[handle].httpDataInfo.mediaFrame.len = httpRequestInfo[handle].httpDataInfo.frameSize;
            httpRequestInfo[handle].httpDataInfo.mediaFrame.videoInfo. frameType = MAX_FRAME_TYPE;
        }
        break;
	}

    if((httpRequestInfo[handle].callback != NULL) && (status == SUCCESS))
	{
		// send it to user as soon as it is received
        (*httpRequestInfo[handle].callback)(handle, &httpRequestInfo[handle].httpDataInfo);
	}

	httpRequestInfo[handle].httpDataInfo.frameSize = 0;
	httpRequestInfo[handle].frameRead = 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function identify the data type of the stream.
 * @param   streamTypeStr
 * @param   reqInfoPtr
 */
static void findHttpStreamType(CHARPTR streamTypeStr, HTTP_REQUEST_INFO_t *reqInfoPtr)
{
    if((strcasecmp(streamTypeStr, HTTP_VIDEO_STREAM) == 0) || (strcasecmp(streamTypeStr, HTTP_IMAGE_STREAM) == 0))
	{
		reqInfoPtr->httpDataInfo.streamType = VIDEO_STREAM;
	}
	else if(strcasecmp(streamTypeStr, HTTP_AUDIO_STREAM) == 0)
	{
		reqInfoPtr->httpDataInfo.streamType = AUDIO_STREAM;
	}
    else if((strcasecmp(streamTypeStr, HTTP_TEXT_STREAM) == 0) || (strcasecmp(streamTypeStr, HTTP_APPLICATION_STREAM) == 0))
	{
		reqInfoPtr->httpDataInfo.streamType = TEXT_STREAM;
	}
	else
	{
		reqInfoPtr->httpDataInfo.streamType = MAX_HTTP_STREAM_TYPE;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will identify the video frame type. Depending upon the codec type it will
 *          give the current vifeo frame type.
 * @param   httpDataInfoPtr
 * @return  TRUE/FALSE
 */
static BOOL getVideoFramePara(HTTP_DATA_INFO_t * httpDataInfoPtr)
{
    UINT8       configPresent = FALSE;
    UINT8PTR    dataPtr = (UINT8PTR)httpDataInfoPtr->storagePtr;

    if ((dataPtr == NULL) || (httpDataInfoPtr->frameSize == 0))
	{
        return FAIL;
    }

    httpDataInfoPtr->mediaFrame.sampleRate = 25;
    httpDataInfoPtr->mediaFrame.len = httpDataInfoPtr->frameSize;
    if(httpDataInfoPtr->mediaFrame.codecType == VIDEO_MJPG)
    {
        return GetJpegSize(dataPtr, httpDataInfoPtr->frameSize, &httpDataInfoPtr->mediaFrame.videoInfo);
    }
    else if(httpDataInfoPtr->mediaFrame.codecType == VIDEO_H264)
    {
        return GetH264Info(dataPtr, httpDataInfoPtr->frameSize, &httpDataInfoPtr->mediaFrame.videoInfo, &configPresent);
    }
    else if(httpDataInfoPtr->mediaFrame.codecType == VIDEO_MPEG4)
    {
        return GetMpeg4Info(dataPtr, httpDataInfoPtr->frameSize, &httpDataInfoPtr->mediaFrame.videoInfo, &configPresent);
    }
    else
    {
        httpDataInfoPtr->mediaFrame.videoInfo.frameType = MAX_FRAME_TYPE;
        httpDataInfoPtr->mediaFrame.videoInfo.width = 0;
        httpDataInfoPtr->mediaFrame.videoInfo.height = 0;
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will parse sampling frequency from the data header.
 * @param   dataPtr
 * @param   reqInfoPtr
 */
static void findSamplingFreq(CHARPTR dataPtr, HTTP_REQUEST_INFO_t *reqInfoPtr)
{
	CHARPTR 			tempPtr, parseFromPtr;
    CHAR				tempBuffer[10] = "";
    UINT8				confByte[10] = "";
	UINT8				confSize;
	AAC_AUDIO_INFO_t	aacAudioInfo;

	parseFromPtr = dataPtr;
    reqInfoPtr->httpDataInfo.mediaFrame.sampleRate = 0;

	switch(reqInfoPtr->httpDataInfo.mediaFrame.codecType)
	{
        case AUDIO_G711_ULAW:
        case AUDIO_G726_8:
        case AUDIO_G726_16:
        case AUDIO_G726_24:
        case AUDIO_G726_32:
        case AUDIO_G726_40:
        case AUDIO_G711_ALAW:
            reqInfoPtr->httpDataInfo.mediaFrame.sampleRate = 8000;
            break;

        case AUDIO_AAC:
            tempPtr = strcasestr(parseFromPtr, AUDIO_FRAME_CONFIG);
            if (NULL == tempPtr)
            {
                EPRINT(HTTP_CLIENT, "invld aac config data");
                break;
            }

            parseFromPtr = (tempPtr + strlen(AUDIO_FRAME_CONFIG));
            tempPtr = strchr(parseFromPtr, ';');
            if (NULL == tempPtr)
            {
                EPRINT(HTTP_CLIENT, "invld aac config data");
                break;
            }

            memcpy(tempBuffer, parseFromPtr, (tempPtr - parseFromPtr));
            tempBuffer[(tempPtr - parseFromPtr)] = '\0';
            if(getByteFromStr(tempBuffer, confByte, &confSize) == SUCCESS)
            {
                if(GetAACAudioInfo(confByte, confSize, &aacAudioInfo) == SUCCESS)
                {
                    reqInfoPtr->httpDataInfo.mediaFrame.sampleRate = aacAudioInfo.samplingFreq;
                    memcpy(reqInfoPtr->audioConfigData, aacAudioInfo.aacHeader, aacAudioInfo.aacHeaderSize);
                    reqInfoPtr->audioConfigDataLen = aacAudioInfo.aacHeaderSize;
                    DPRINT(HTTP_CLIENT, "aac sampling freq: [rate=%d]", reqInfoPtr->httpDataInfo.mediaFrame.sampleRate);
                }
                else
                {
                    EPRINT(HTTP_CLIENT, "invld aac config byte received");
                }
            }
            else
            {
                EPRINT(HTTP_CLIENT, "invld aac config data");
            }
            break;

        case AUDIO_PCM_L:
        case AUDIO_PCM_B:
            reqInfoPtr->httpDataInfo.mediaFrame.sampleRate = 8000;
            break;

        default:
            break;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will take in the string as argument and converts it into the HEX data.
 * @param   str
 * @param   confByte
 * @param   confSize
 * @return
 */
static BOOL getByteFromStr(CHARPTR str, UINT8PTR confByte, UINT8PTR confSize)
{
    UINT8 nibble, firstNibble, secondNibble;

	for(nibble = 0, (*confSize) = 0 ; nibble < strlen(str) ; (*confSize)++)
	{
        if(FAIL == getNibble(&str[nibble++], &firstNibble))
		{
            break;
        }

        if(FAIL == getNibble(&str[nibble++], &secondNibble))
        {
            break;
        }

        confByte[(*confSize)] = ((firstNibble << 4) | secondNibble);
	}

	if(nibble < strlen(str))
	{
        return FAIL;
	}

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will take in the character as argument and converts it into the HEX.
 * @param   srcChr
 * @param   resNbl
 * @return  SUCCESS/FAIL
 */
static BOOL getNibble(CHARPTR srcChr, UINT8PTR resNbl)
{
	CHAR 	chr = *(srcChr);
	UINT8	resultNibble;

	if((chr >= '0') && (chr <= '9'))
	{
	   resultNibble = chr - '0';
	}
	else if((chr >= 'A') && (chr <= 'F'))
	{
	   resultNibble = 10 + chr - 'A';
	}
	else if((chr >= 'a') && (chr <= 'f'))
	{
	   resultNibble = 10 + chr - 'a';
	}
	else
	{
       return FAIL;
	}

	(*resNbl) = resultNibble;
	return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   findContentLength
 * @param   data
 * @param   lengthPtr
 * @return
 */
static BOOL findContentLength(CHARPTR data, UINT32PTR lengthPtr)
{
    UINT8   cnt;
    UINT32  length;
    UINT64  contentLen;
    CHAR    tempString[20];
    CHARPTR tempStr;

	for(cnt = 0; cnt < CONTENT_LENGTH_TAG_MAX; cnt++)
	{
		tempStr = contentLengthTag[cnt];
		length = strlen(tempStr);
        if(strncasecmp(data, tempStr, length) != STATUS_OK)
		{
            continue;
        }

        data += length;
        while((*data == ':') || (*data == ' '))
        {
            data++;
        }

        if((tempStr = strstr(data, CR_LF_STR)) == NULL)
        {
            return FAIL;
        }

        length = (tempStr - data);
        memcpy(tempString, data, length);
        tempString[length] = '\0';
        if(FAIL == AsciiToInt(tempString, &contentLen))
        {
            return FAIL;
        }

        *lengthPtr = (UINT32)contentLen;
        return SUCCESS;
	}

    return FAIL;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
