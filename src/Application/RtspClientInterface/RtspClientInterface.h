#if !defined RTSP_CLIENT_INTERFACE_H
#define RTSP_CLIENT_INTERFACE_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		RtspClientInterface.h
@brief      This Module is used for interfacing RTSP client applications.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/un.h>

/* Application Includes */
#include "NetworkManager.h"
#include "Utils.h"
#include "DebugLog.h"
#include "VideoParser.h"
#include "DeviceDefine.h"

/***********************************************************************************************
* @DEFINE
***********************************************************************************************/
/* Unix socket for interfacing with RTSP client application */
#define RTSP_CLIENT_UNIX_SOCKET     "/tmp/rtspclientsock"
#define UNIX_SOCK_TIMEOUT_IN_SEC    1

#define SHM_NAME_VIDEO              "/shmMemV"
#define SHM_NAME_AUDIO              "/shmMemA"

/* Define number for RTSP client applications. It should be in multiple of max media session */
#if (MAX_CAMERA == 96)
    #define RTSP_CLIENT_APPL_COUNT  6
#elif (MAX_CAMERA == 64)
    #define RTSP_CLIENT_APPL_COUNT  4
#elif (MAX_CAMERA == 16)
#if defined(RK3568_NVRL)
    #define RTSP_CLIENT_APPL_COUNT  2   /* It takes high CPU usage in Rockchip SoC. To distribute load equally in cores, increased processes */
#else
    #define RTSP_CLIENT_APPL_COUNT  1
#endif
#endif

/* Define maximum media session to be supported */
#define RTSP_MEDIA_SESSION_MAX      (MAX_CAMERA * 2)

/* Media session limit per application */
#define MEDIA_SESSION_PER_RTSP_APPL (RTSP_MEDIA_SESSION_MAX / RTSP_CLIENT_APPL_COUNT)

/* Increase VIDEO_BUF_SIZE if frame size exceed it */
#define VIDEO_BUF_SIZE_MEDIA        (2 * MEGA_BYTE)
#define VIDEO_BUF_SIZE              (VIDEO_BUF_SIZE_MEDIA + sizeof(MEDIA_FRAME_INFO_t))

/* Audio buffer size */
#define	AUDIO_BUF_SIZE_MEDIA        (2 * KILO_BYTE)
#define AUDIO_BUF_SIZE              (AUDIO_BUF_SIZE_MEDIA + sizeof(MEDIA_FRAME_INFO_t))

/***********************************************************************************************
* @ENUM
***********************************************************************************************/
typedef UINT8 RTSP_HANDLE;

typedef enum
{
    OVER_UDP,
    TCP_INTERLEAVED,
    HTTP_TUNNELING
}TRANSPORT_TYPE_e;

typedef enum
{
    MSG_TYPE_REQUEST = 0,
    MSG_TYPE_RESPONSE,
    MSG_TYPE_EVENT,
    MSG_TYPE_MAX,
}MsgType_e;

/* RTSP response code for each frame */
typedef enum
{
    RTSP_RESP_CODE_CONNECT_FAIL = 0,
    RTSP_RESP_CODE_CONN_CLOSE,
    RTSP_RESP_CODE_CONFIG_VIDEO_DATA,
    RTSP_RESP_CODE_CONFIG_AUDIO_DATA,
    RTSP_RESP_CODE_VIDEO_DATA,
    RTSP_RESP_CODE_AUDIO_DATA,
    RTSP_RESP_CODE_FRAME_TIMEOUT,
    RTSP_RESP_CODE_MAX
}MediaFrameResp_e;

/* Message ID used to communicate with RTSP client */
typedef enum
{
    RTSP_CLIENT_MSG_ID_REGISTER = 0,
    RTSP_CLIENT_MSG_ID_DEBUG_CONFIG,
    RTSP_CLIENT_MSG_ID_START_STREAM,
    RTSP_CLIENT_MSG_ID_STOP_STREAM,
    RTSP_CLIENT_MSG_ID_MEDIA_FRAME,
    RTSP_CLIENT_MSG_ID_ALLOC_RTP_PORT,
    RTSP_CLIENT_MSG_ID_DEALLOC_RTP_PORT,
    RTSP_CLIENT_MSG_ID_MAX
}RtspClientMsgId_e;

/***********************************************************************************************
* @STRUCT
***********************************************************************************************/
typedef struct
{
    UINT8				camIndex;
    TRANSPORT_TYPE_e	transport;
    CHAR				ip[IPV6_ADDR_LEN_MAX];
    UINT16				port;
    CHAR 				url[MAX_CAMERA_URL_WIDTH];
    CHAR 				usrname[MAX_CAMERA_USERNAME_WIDTH];
    CHAR	 			pswd[MAX_CAMERA_PASSWORD_WIDTH];
}RtspStreamInfo_t;

typedef struct
{
    RTSP_HANDLE         mediaHandle;
    RtspStreamInfo_t    rtspStreamInfo;
}StartStreamInfo_t;

typedef struct
{
    MediaFrameResp_e    response;
    UINT8               camIndex;
    RTSP_HANDLE         mediaHandle;
    UINT32              headSize;
    UINT32              offset;
}MediaFrameInfo_t;

typedef struct
{
    RtspClientMsgId_e   msgId;
    MsgType_e           msgType;
    NET_CMD_STATUS_e    status;
}RtspClientHeader_t;

typedef union
{
    RTSP_HANDLE         mediaHandle;
    UINT16              rtpPort;
    StartStreamInfo_t   startStreamInfo;
    MediaFrameInfo_t    mediaFrameInfo;
}RtspClientPayload_u;

typedef struct
{
    RtspClientHeader_t header;
    RtspClientPayload_u payload;

}RtspClientMsgInfo_t;

/***********************************************************************************************
* @FUNCTION PROTOTYPE
***********************************************************************************************/
//---------------------------------------------------------------------------------------------
typedef void (*RTSP_CALLBACK) (MediaFrameResp_e, UINT8PTR, MEDIA_FRAME_INFO_t *, UINT8);
//---------------------------------------------------------------------------------------------
void InitRtspClientInterface(void);
//---------------------------------------------------------------------------------------------
void DeinitRtspClientInterface(void);
//---------------------------------------------------------------------------------------------
void SetRtspClientDebugConfig(void);
//---------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartRtspStream(RtspStreamInfo_t *rtspInfo, RTSP_CALLBACK callBack, RTSP_HANDLE *rtspHandle);
//---------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StopRtspStream(UINT8 camIndex, RTSP_HANDLE rtspHandle);
//---------------------------------------------------------------------------------------------
/***********************************************************************************************
* @END OF FILE
***********************************************************************************************/
#endif // RTSP_CLIENT_INTERFACE_H


