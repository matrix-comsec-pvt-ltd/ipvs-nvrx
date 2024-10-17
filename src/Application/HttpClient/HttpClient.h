#if !defined HTTPCLIENT_H
#define HTTPCLIENT_H
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
/* Library Includes */
#include <curl/curl.h>

/* Application Includes */
#include "Config.h"
#include "VideoParser.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_HTTP_REQUEST		(MAX_CAMERA * 5)    // maximum number of acceptable request
#define INVALID_HTTP_HANDLE		(0xFFFF)            // invalid HTTP handle

#define MAX_RELATIVE_URL_WIDTH  (512)
#define HTTP_CONNECTION_TIMEOUT (5)		// connection timeout in seconds
#define HTTP_FRAME_TIMEOUT      (10)	// 10 Second
#define MAX_FILE_NAME_LENGTH    (150)	// string of 150 char

//#################################################################################################
// @DATA_TYPES
//#################################################################################################
typedef UINT16 HTTP_HANDLE;

typedef enum
{
    HTTP_CLOSE_ON_SUCCESS = 0,
    HTTP_CLOSE_ON_ERROR,
    HTTP_SUCCESS,
    HTTP_ERROR,
    MAX_HTTP_RESPONSE
}HTTP_RESPONSE_e;

typedef enum
{
    VIDEO_STREAM,
    AUDIO_STREAM,
    TEXT_STREAM,
    MAX_HTTP_STREAM_TYPE
}HTTP_STREAM_TYPE_e;

typedef enum
{
	AUTH_TYPE_NONE = 0,
	AUTH_TYPE_BASIC,
	AUTH_TYPE_DIGEST,
	AUTH_TYPE_ANY,
	MAX_AUTH_TYPE
}REQUEST_AUTH_TYPE_e;

typedef enum
{
	INTERFACE_NONE = 0,
    INTERFACE_USB_MODEM,
	MAX_HTTP_INTERFACE,

}HTTP_INTERFACE_e;

typedef enum
{
	CURL_USER_AGENT = 0,
	NS_HTTP_USER_AGENT,
	MAX_HTTP_USER_AGENT,

}HTTP_USER_AGENT_e;

typedef enum
{
    HTTP_CONTENT_TYPE_NONE = 0,
    HTTP_CONTENT_TYPE_XML,
    HTTP_CONTENT_TYPE_MAX

}HTTP_CONTENT_TYPE_e;

typedef enum
{
    GET_REQUEST = 0,
    POST_REQUEST,
    PUT_REQUEST,
    DELETE_REQUEST,
    PUT_REBOOT_REQUEST,
    MAX_HTTP_REQUEST_TYPE,

}HTTP_REQUEST_e;

typedef struct
{
    CHAR 				username[MAX_CAMERA_USERNAME_WIDTH];
    CHAR 				password[MAX_CAMERA_PASSWORD_WIDTH];
}HTTP_USRPWD_t;

typedef struct
{
	CHAR				ipAddress[MAX_CAMERA_ADDRESS_WIDTH];
	UINT16				port;
	CHAR 				relativeUrl[MAX_RELATIVE_URL_WIDTH];
	CHAR				fileForPutReq[MAX_FILE_NAME_LENGTH];
	UINT32				sizeOfPutFile;
	HTTP_USRPWD_t 		httpUsrPwd;
	REQUEST_AUTH_TYPE_e authMethod;
	UINT8				maxConnTime;
	UINT8				maxFrameTime;
	HTTP_INTERFACE_e	interface;
	HTTP_USER_AGENT_e	userAgent;
    HTTP_CONTENT_TYPE_e contentType;

}HTTP_INFO_t;

typedef struct
{
    HTTP_RESPONSE_e 		httpResponse;	// Server response
    NET_CMD_STATUS_e		cmdResponse;
    HTTP_STREAM_TYPE_e		streamType;		// Stream type
    VOIDPTR 				storagePtr;		// Head pointer to store buffer
    UINT32	 				ptrSize;		// size to store buffer
    UINT32					frameSize;
    UINT32					userData;		// user data send
    MEDIA_FRAME_INFO_t		mediaFrame;
}HTTP_DATA_INFO_t;

//-------------------------------------------------------------------------------------------------
typedef void (*HTTP_CALLBACK)(HTTP_HANDLE httpHandle, HTTP_DATA_INFO_t *dataInfo);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitHttp(void);
//-------------------------------------------------------------------------------------------------
BOOL StartHttp(HTTP_REQUEST_e httpRequest, HTTP_INFO_t * httpInfo, HTTP_CALLBACK callback, UINT32 userData, HTTP_HANDLE * handlePtr);
//-------------------------------------------------------------------------------------------------
BOOL StopHttp(HTTP_HANDLE httpHandle);
//-------------------------------------------------------------------------------------------------
REQUEST_AUTH_TYPE_e GetAuthType(HTTP_HANDLE httpHandle);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // HTTPCLIENT_H
