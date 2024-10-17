// #################################################################################################
//  FILE BRIEF
// #################################################################################################
/**
@file		TcpClient.c
@brief      This module provides TCP communication functionality for camera initiation.
*/
// #################################################################################################
//  @INCLUDES
// #################################################################################################
/* OS Includes */
#include <sys/resource.h>

/* Application Includes */
#include "AutoConfigCamera.h"
#include "DebugLog.h"
#include "MediaStreamer.h"
#include "TcpClient.h"
#include "Utils.h"

// #################################################################################################
//  @DEFINES
// #################################################################################################
#define CI_MAX_COMM_DATA                    (512)

#define CI_FRAME_HEADER_LEN                 (40)
#define CI_MAX_FRAME_LEN                    (MEGA_BYTE * 2)
#define CAM_PRODUCT_TYPE                    (3)

#define MAX_CI_RESOLUTION                   (95)
#define MAX_TIMEOUT_STREAM                  (200000)
#define DEFAULT_REF_NUM                     (2)

#define MAX_IMG_LEN                         (10)
#define MAX_LV_CMD_TIMEOUT                  (5000)

#define MAX_LEN_IMG_RCV                     (1000000)
#define MAX_SNAP_SHOT_SIZE                  (2000000)
#define MAX_SEND_LEN_SOCK                   (1024)

#define MAX_RVC_LEN_SOCK                    (1000000)
#define MAX_MAGIC_CODE_LEN                  (4)
#define MAX_LOOP_ALLOW                      (1000)

#define CI_LV_CMD_SND_RCV                   (1)
#define INVALID_HEIGHT_WIDTH                (5000)

#define MAX_LIVE_STRM_REQ                   (4)

#define IMG_TRANSFER_THREAD_STACK_SZ        (4 * MEGA_BYTE)
#define TCP_TRANSFER_THREAD_STACK_SZ        (4 * MEGA_BYTE)
#define TCP_LIVESTREAM_THREAD_STACK_SZ      (4 * MEGA_BYTE)
#define TCP_LIVE_STREAM_CMD_THREAD_STACK_SZ (4 * MEGA_BYTE)

#define CI_RESET_LIVE_STREAM_STS(strm, camIdx, video)   \
    MUTEX_LOCK(liveStreamInfo[strm].mutex);             \
    liveStreamInfo[strm].status[video][camIdx] = FALSE; \
    MUTEX_UNLOCK(liveStreamInfo[strm].mutex);

// #################################################################################################
//  @DATA TYPES
// #################################################################################################
typedef enum
{
    CI_IMAGE_SETTING_CAP_BRIGHTNESS = 0,
    CI_IMAGE_SETTING_CAP_CONTRAST,
    CI_IMAGE_SETTING_CAP_SATURATION,
    CI_IMAGE_SETTING_CAP_HUE,
    CI_IMAGE_SETTING_CAP_SHARPNESS,
    CI_IMAGE_SETTING_CAP_WHITE_BALANCE = 9,
    CI_IMAGE_SETTING_CAP_WDR = 16,
    CI_IMAGE_SETTING_CAP_EXPOSURE_RATIO,
    CI_IMAGE_SETTING_CAP_EXPOSURE_RATIO_MODE = 19,
    CI_IMAGE_SETTING_CAP_BACKLIGHT,
    CI_IMAGE_SETTING_CAP_DWDR_STRENGTH = 21,
    CI_IMAGE_SETTING_CAP_FLICKER,
    CI_IMAGE_SETTING_CAP_EXPOSURE_MODE,
    CI_IMAGE_SETTING_CAP_EXPOSURE_GAIN = 25,
    CI_IMAGE_SETTING_CAP_EXPOSURE_IRIS,
    CI_IMAGE_SETTING_CAP_NORMAL_LIGHT_GAIN,
    CI_IMAGE_SETTING_CAP_NORMAL_LIGHT_LUMINANCE,
    CI_IMAGE_SETTING_CAP_LED_MODE = 37,
    CI_IMAGE_SETTING_CAP_LED_SENSITIVITY = 43,
    CI_IMAGE_SETTING_CAP_FIELD_MAX,
} CI_IMAGE_SETTING_CAP_FIELD_e;

typedef enum
{
    CI_STREAM_PROFILE_CFG_PARSER = 0,
    CI_OVERLAY_CFG_PARSER,
    CI_MOTION_DETECT_CFG_PARSER,
    CI_APPEARANCE_CFG_PARSER,
    CI_ADV_APPEARANCE_CFG_PARSER,
    CI_DAY_NIGHT_CFG_PARSER,
    MAX_CI_CFG_TABLE_PARSER,
} CI_CNFG_PARSER_e;

typedef enum
{
    CI_CMD_PARSER_IMAGE_SETTING = 0,
    CI_CMD_PARSER_MAX
} CI_CMD_PARSER_e;

typedef enum
{
    CMD_STREAM = 0,
    LIVE_STREAM,
    MAX_LIVE_STREAM_TID,
} TYPE_REQ_e;

typedef enum
{
    FRAME_RECV_STATE_FIRST_FRAME = 0,
    FRAME_RECV_STATE_MAGIC_CODE,
    FRAME_RECV_STATE_NEW_HEADER,
    FRAME_RECV_STATE_LEFT_HEADER,
    FRAME_RECV_STATE_NEW_PAYLOAD,
    FRAME_RECV_STATE_LEFT_PAYLOAD,
    FRAME_RECV_STATE_MAX,
} FRAME_RECV_STATE_e;

typedef enum
{
    CAM_CMD_STATE_INIT = 0,
    CAM_CMD_STATE_HANDLE_GOT,
    CAM_CMD_STATE_FD_GOT,
    CAM_CMD_STATE_FD_WAIT,
    CAM_CMD_STATE_MAX
} CAM_CMD_STATE_e;

typedef enum
{
    NONE_STREAM_STATE = 0,
    CMD_STREAM_STATE,
    LIVE_STREAM_STATE,
    RUN_STREAM_STATE,
    STOP_STREAM_STATE,
    MAX_STREAM_STATE
} TYPE_STREAM_STATE_e;

typedef struct
{
    BOOL           requestStatus;
    TCP_HANDLE     tcpHandle;
    TCP_REQ_INFO_t tcpInfo;
    TCP_CALLBACK   tcpResponseCb;
} TCP_CLIENT_INFO_t;

typedef struct
{
    UINT8               streamReqCnt;
    TYPE_STREAM_STATE_e streamState;
    pthread_mutex_t     streamMutex;
    UINT8               profileNum;
    INT32               tcpCommFd;
    UINT32              tcpCommFdQueueIdx;
    TCP_STREAM_CALLBACK tcpFrameCb;
    pthread_mutex_t     tcpCamLock;
} STREAM_CAM_COMM_INFO_t;

typedef struct
{
    BOOL            status[MAX_STREAM][MAX_CAMERA];
    pthread_mutex_t mutex;
    pthread_cond_t  signal;
} LIVE_STREAM_t;

typedef struct
{
    BOOL               frameRecvF;
    UINT32             timeoutCnt;
    FRAME_RECV_STATE_e frameState;
    UINT8              headerData[CI_FRAME_HEADER_LEN];
    UINT8PTR           headerRef;
    UINT8PTR           payloadData;
    UINT8PTR           payloadRef;
    UINT32             payloadLen;
} FRAME_DATA_INFO_t;

// #################################################################################################
//  @GLOBAL VARIABLES
// #################################################################################################
const CHARPTR CIReqHeader[MAX_TCP_REQUEST_TYPE] = {
    "SET_CFG",           // index 0
    "GET_CFG",           // index 1
    "SRT_LV_STRM",       // index 2
    "STP_LV_STRM",       // index 3
    "INC_LV_AUD",        // index 4
    "EXC_LV_AUD",        // index 5
    "BITRATE_RES",       // index 6
    "SNP_SHT",           // index 7
    "OTHR_SUPP",         // index 8
    "ALRM_OUT",          // index 9
    "MAX_PRIVACY_MASK",  // index 10
    "RPL_CMD",           // index 11
    "RPL_CFG",           // index 12
    "SET_CMD",           // index 13
    "SETFOCUS",          // index 14
    "SETZOOM",           // index 15
    "SET_DATE_TIME",     // index 16
    "SRT_AUD_OUT",       // index 17
    "STP_AUD_OUT",       // index 18
    "IMGSET_CAP",        // index 19
    "SET_PANTILT",       // index 20
    "UPDATE_PRESET",     // index 21
    "CALL_PRESET",       // index 22
};

static const UINT16 frameResolution[MAX_CI_RESOLUTION][MAX_FRAME_DIMENSION] = {
    {0,    0   }, // resolution 00
    {160,  90  }, // resolution 01
    {160,  100 }, // resolution 02
    {160,  112 }, // resolution 03
    {160,  120 }, // resolution 04
    {160,  128 }, // resolution 05
    {176,  112 }, // resolution 06
    {176,  120 }, // resolution 07
    {176,  144 }, // resolution 08
    {192,  144 }, // resolution 09
    {192,  192 }, // resolution 10
    {240,  135 }, // resolution 11
    {240,  180 }, // resolution 12
    {256,  192 }, // resolution 13
    {256,  256 }, // resolution 14
    {320,  180 }, // resolution 15
    {320,  192 }, // resolution 16
    {320,  200 }, // resolution 17
    {320,  240 }, // resolution 18
    {320,  256 }, // resolution 19
    {320,  320 }, // resolution 20
    {352,  240 }, // resolution 21
    {352,  244 }, // resolution 22
    {352,  288 }, // resolution 23
    {384,  216 }, // resolution 24
    {384,  288 }, // resolution 25
    {384,  384 }, // resolution 26
    {480,  270 }, // resolution 27
    {480,  300 }, // resolution 28
    {480,  360 }, // resolution 29
    {512,  384 }, // resolution 30
    {512,  512 }, // resolution 31
    {528,  320 }, // resolution 32
    {528,  328 }, // resolution 33
    {640,  360 }, // resolution 34
    {640,  368 }, // resolution 35
    {640,  400 }, // resolution 36
    {640,  480 }, // resolution 37
    {640,  512 }, // resolution 38
    {704,  240 }, // resolution 39
    {704,  288 }, // resolution 40
    {704,  480 }, // resolution 41
    {704,  570 }, // resolution 42
    {704,  576 }, // resolution 43
    {720,  480 }, // resolution 44
    {720,  576 }, // resolution 45
    {768,  576 }, // resolution 46
    {768,  768 }, // resolution 47
    {800,  450 }, // resolution 48
    {800,  480 }, // resolution 49
    {800,  500 }, // resolution 50
    {800,  600 }, // resolution 51
    {860,  540 }, // resolution 52
    {960,  540 }, // resolution 53
    {960,  720 }, // resolution 54
    {960,  768 }, // resolution 55
    {1024, 576 }, // resolution 56
    {1024, 640 }, // resolution 57
    {1024, 768 }, // resolution 58
    {1056, 1056}, // resolution 59
    {1140, 1080}, // resolution 60
    {1280, 720 }, // resolution 61
    {1280, 800 }, // resolution 62
    {1280, 960 }, // resolution 63
    {1280, 1024}, // resolution 64
    {1280, 1280}, // resolution 65
    {1286, 972 }, // resolution 66
    {1296, 968 }, // resolution 67
    {1296, 972 }, // resolution 68
    {1360, 768 }, // resolution 69
    {1376, 768 }, // resolution 70
    {1440, 912 }, // resolution 71
    {1472, 960 }, // resolution 72
    {1536, 1536}, // resolution 73
    {1600, 904 }, // resolution 74
    {1600, 912 }, // resolution 75
    {1600, 1200}, // resolution 76
    {1680, 1056}, // resolution 77
    {1824, 1376}, // resolution 78
    {1920, 1080}, // resolution 79
    {1920, 1200}, // resolution 80
    {1920, 1440}, // resolution 81
    {2032, 1920}, // resolution 82
    {2048, 1536}, // resolution 83
    {2560, 1600}, // resolution 84
    {2560, 1920}, // resolution 85
    {2592, 1944}, // resolution 86
    {2944, 1920}, // resolution 87
    {3648, 2752}, // resolution 88
    {176,  128 }, // resolution 89
    {720,  240 }, // resolution 90
    {720,  288 }, // resolution 91
    {2592, 1520}, // resolution 92
    {3200, 1800}, // resolution 93
    {3840, 2160}, // resolution 94
};

// #################################################################################################
//  @STATIC VARIABLES
// #################################################################################################
static BOOL                   threadExitF = FALSE;
static STREAM_CAM_COMM_INFO_t liveStreamCommInfo[MAX_STREAM][MAX_CAMERA];

static pthread_mutex_t   tcpReqListMutex;
static TCP_CLIENT_INFO_t tcpClientInfo[MAX_TCP_REQUEST];
static TCP_DATA_INFO_t   tcpRespData[MAX_TCP_REQUEST];
static LIVE_STREAM_t     liveStreamInfo[MAX_LIVE_STREAM_TID];

// #################################################################################################
//  @PROTOTYPES
// #################################################################################################
//-------------------------------------------------------------------------------------------------
static VOIDPTR tcpTransfer(VOIDPTR tcpSessionPtr);
//-------------------------------------------------------------------------------------------------
static VOIDPTR tcpTransferImage(VOIDPTR tcpSessionPtr);
//-------------------------------------------------------------------------------------------------
static VOIDPTR tcpLiveStreamCmd(VOIDPTR tcpSessionPtr);
//-------------------------------------------------------------------------------------------------
static VOIDPTR tcpLiveStream(VOIDPTR tcpSessionPtr);
//-------------------------------------------------------------------------------------------------
static void resetTcpLiveStreamFrameInfo(FRAME_DATA_INFO_t *pFrameInfo);
//-------------------------------------------------------------------------------------------------
static CI_CNFG_PARSER_e getCnfgParserIndex(UINT64 tableId);
//-------------------------------------------------------------------------------------------------
static BOOL streamProfileCnfgParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle);
//-------------------------------------------------------------------------------------------------
static BOOL overlayCnfgParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle);
//-------------------------------------------------------------------------------------------------
static BOOL motionCnfgParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle);
//-------------------------------------------------------------------------------------------------
static BOOL appearanceCnfgParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle);
//-------------------------------------------------------------------------------------------------
static BOOL advanceAppearanceCnfgParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle);
//-------------------------------------------------------------------------------------------------
static BOOL dayNightCnfgParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle);
//-------------------------------------------------------------------------------------------------
static CI_CMD_PARSER_e getCmdParserIndex(UINT64 cmdId);
//-------------------------------------------------------------------------------------------------
static BOOL imageSettingCapabilityCmdParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle);
//-------------------------------------------------------------------------------------------------
static UINT32 increaseSocketBufferSize(INT32 bufOptName, INT32 socket, UINT32 requestedSize);
//-------------------------------------------------------------------------------------------------
static UINT32 getBufferSize(INT32 bufOptName, INT32 socket);
//-------------------------------------------------------------------------------------------------
static void sendStopToCameraInterface(UINT8 camIndex, UINT8 streamType);
//-------------------------------------------------------------------------------------------------
static void StopCameraStream(UINT8 camIndex, UINT8 streamType, BOOL force);
//-------------------------------------------------------------------------------------------------
static BOOL isAnyCameraAddedForLiveStream(BOOL liveStreamStatusF[MAX_STREAM][MAX_CAMERA]);
//-------------------------------------------------------------------------------------------------
static BOOL (*ciCnfgRespParserFuncPtr[MAX_CI_CFG_TABLE_PARSER])(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle) = {
    streamProfileCnfgParser,      // stream config parser
    overlayCnfgParser,            // overlay config parser
    motionCnfgParser,             // motion config parser
    appearanceCnfgParser,         // image setting appearance config parser
    advanceAppearanceCnfgParser,  // image setting advance appearance config parser
    dayNightCnfgParser,           // image setting day-night config parser
};
//-------------------------------------------------------------------------------------------------
static BOOL (*ciCmdRespParserFuncPtr[CI_CMD_PARSER_MAX])(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle) = {
    imageSettingCapabilityCmdParser,  // image setting capability command parser
};
//-------------------------------------------------------------------------------------------------
// #################################################################################################
// @FUNCTIONS
// #################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API Initializes TCP module.
 */
void InitTcpClient(void)
{
    TYPE_REQ_e reqType;
    TCP_HANDLE handle;
    UINT8      camIndex, streamType;

    MUTEX_INIT(tcpReqListMutex, NULL);

    for (handle = 0; handle < MAX_TCP_REQUEST; handle++)
    {
        tcpClientInfo[handle].requestStatus = FREE;
        tcpClientInfo[handle].tcpHandle = handle;
        memset(&tcpRespData[handle], 0, sizeof(TCP_DATA_INFO_t));
    }

    for (streamType = 0; streamType < MAX_STREAM; streamType++)
    {
        for (camIndex = 0; camIndex < MAX_CAMERA; camIndex++)
        {
            liveStreamCommInfo[streamType][camIndex].tcpCommFd = INVALID_CONNECTION;
            liveStreamCommInfo[streamType][camIndex].tcpFrameCb = NULL;
            MUTEX_INIT(liveStreamCommInfo[streamType][camIndex].streamMutex, NULL);
            liveStreamCommInfo[streamType][camIndex].streamState = NONE_STREAM_STATE;
            liveStreamCommInfo[streamType][camIndex].streamReqCnt = 0;
        }
    }

    for (reqType = 0; reqType < MAX_LIVE_STREAM_TID; reqType++)
    {
        MUTEX_INIT(liveStreamInfo[reqType].mutex, NULL);
        for (streamType = 0; streamType < MAX_STREAM; streamType++)
        {
            for (camIndex = 0; camIndex < MAX_CAMERA; camIndex++)
            {
                liveStreamInfo[reqType].status[streamType][camIndex] = FALSE;
            }
        }
    }

#if !defined(OEM_JCI)
    /* Create TCP live stream command thread */
    if (FALSE == Utils_CreateThread(NULL, tcpLiveStreamCmd, NULL, DETACHED_THREAD, TCP_LIVE_STREAM_CMD_THREAD_STACK_SZ))
    {
        EPRINT(CAMERA_INITIATION, "Failed to create tcp live stream cmd thread");
        return;
    }

    /* Create TCP live stream thread */
    if (FALSE == Utils_CreateThread(NULL, tcpLiveStream, NULL, DETACHED_THREAD, TCP_LIVESTREAM_THREAD_STACK_SZ))
    {
        EPRINT(CAMERA_INITIATION, "Failed to create tcp live stream thread");
        return;
    }
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API deinitialize TCP module.
 */
void DeInitTcpClient(void)
{
    threadExitF = TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API Stops Camera Stream.
 * @param   camIndex
 * @param   streamType
 * @param   force
 */
static void StopCameraStream(UINT8 camIndex, UINT8 streamType, BOOL force)
{
    CI_RESET_LIVE_STREAM_STS(CMD_STREAM, camIndex, streamType);

    MUTEX_LOCK(liveStreamInfo[LIVE_STREAM].mutex);
    if (TRUE == liveStreamInfo[LIVE_STREAM].status[streamType][camIndex])
    {
        liveStreamInfo[LIVE_STREAM].status[streamType][camIndex] = FALSE;
        pthread_cond_signal(&liveStreamInfo[LIVE_STREAM].signal);
    }
    MUTEX_UNLOCK(liveStreamInfo[LIVE_STREAM].mutex);

    if (TRUE == force)
    {
        sendStopToCameraInterface(camIndex, streamType);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API starts communication with HTTP server
 * @param   tcpInfo
 * @param   callback
 * @param   handlePtr
 * @param   arg
 * @return  SUCCESS/FAIL
 */
NET_CMD_STATUS_e StartTcp(TCP_REQ_INFO_t *tcpInfo, VOIDPTR callback, TCP_HANDLE *handlePtr, VOIDPTR arg)
{
    TCP_HANDLE handle = MAX_TCP_REQUEST;
    UINT8      streamType = GET_STREAM_TYPE(tcpInfo->camIndex);
    UINT8      camIndex = GET_STREAM_INDEX(tcpInfo->camIndex);

    if (tcpInfo->tcpRequest >= MAX_TCP_REQUEST_TYPE)
    {
        EPRINT(CAMERA_INITIATION, "invld tcp request type: [camera=%d], [tcpRequest=%d]", tcpInfo->camIndex, tcpInfo->tcpRequest);
        return CMD_PROCESS_ERROR;
    }

    if (CI_SRT_LV_STRM == tcpInfo->tcpRequest)
    {
        MUTEX_LOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
        if (NONE_STREAM_STATE != liveStreamCommInfo[streamType][camIndex].streamState)
        {
            MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
            WPRINT(CAMERA_INITIATION, "invld state in start live stream: [camera=%d], [stream=%d], [state=%d]", camIndex, streamType,
                   liveStreamCommInfo[streamType][camIndex].streamState);
            liveStreamCommInfo[streamType][camIndex].streamReqCnt++;
            if (liveStreamCommInfo[streamType][camIndex].streamReqCnt >= MAX_LIVE_STRM_REQ)
            {
                liveStreamCommInfo[streamType][camIndex].streamReqCnt = 0;
                StopCameraStream(camIndex, streamType, TRUE);
            }
            return CMD_PROCESS_ERROR;
        }
        MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);

        liveStreamCommInfo[streamType][camIndex].streamReqCnt = 0;
        MUTEX_LOCK(liveStreamCommInfo[streamType][camIndex].tcpCamLock);
        liveStreamCommInfo[streamType][camIndex].profileNum = *(UINT8PTR)arg;
        liveStreamCommInfo[streamType][camIndex].tcpFrameCb = (TCP_STREAM_CALLBACK)callback;
        MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].tcpCamLock);

        MUTEX_LOCK(liveStreamInfo[CMD_STREAM].mutex);
        if (TRUE == liveStreamInfo[CMD_STREAM].status[streamType][camIndex])
        {
            /* Command in progress */
            MUTEX_UNLOCK(liveStreamInfo[CMD_STREAM].mutex);
            WPRINT(CAMERA_INITIATION, "start live stream request in-progress: [camera=%d], [stream=%d], [state=%d]", camIndex, streamType,
                   liveStreamCommInfo[streamType][camIndex].streamState);
            return CMD_PROCESS_ERROR;
        }
        MUTEX_UNLOCK(liveStreamInfo[CMD_STREAM].mutex);

        if (NULL != handlePtr)
        {
            *handlePtr = 0;
        }
        MUTEX_LOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
        liveStreamCommInfo[streamType][camIndex].streamState = CMD_STREAM_STATE;
        MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);

        MUTEX_LOCK(liveStreamInfo[CMD_STREAM].mutex);
        liveStreamInfo[CMD_STREAM].status[streamType][camIndex] = TRUE;
        pthread_cond_signal(&liveStreamInfo[CMD_STREAM].signal);
        MUTEX_UNLOCK(liveStreamInfo[CMD_STREAM].mutex);
        DPRINT(CAMERA_INITIATION, "start request send: [camera=%d], [stream=%d], [state=%d]", camIndex, streamType,
               liveStreamCommInfo[streamType][camIndex].streamState);
        return CMD_SUCCESS;
    }

    /* Check free session available or not */
    MUTEX_LOCK(tcpReqListMutex);
    for (handle = 0; handle < MAX_TCP_REQUEST; handle++)
    {
        if (FREE == tcpClientInfo[handle].requestStatus)
        {
            tcpClientInfo[handle].requestStatus = BUSY;
            break;
        }
    }
    MUTEX_UNLOCK(tcpReqListMutex);

    if (handle >= MAX_TCP_REQUEST)
    {
        EPRINT(CAMERA_INITIATION, "tcp client max session reached: [camera=%d], [stream=%d], [tcpRequest=%d]", camIndex, streamType,
               tcpInfo->tcpRequest);
        return CMD_RESOURCE_LIMIT;
    }

    memset(&tcpRespData[handle], 0, sizeof(TCP_DATA_INFO_t));
    if (tcpInfo->tcpRequest == CI_STP_LV_STRM)
    {
        if ((camIndex >= getMaxCameraForCurrentVariant()) || (streamType >= MAX_STREAM))
        {
            /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
            MUTEX_LOCK(tcpReqListMutex);
            tcpClientInfo[handle].requestStatus = FREE;
            MUTEX_UNLOCK(tcpReqListMutex);
            EPRINT(CAMERA_INITIATION, "invld camera index in stop stream: [camera=%d], [stream=%d]", camIndex, streamType);
            return CMD_PROCESS_ERROR;
        }

        if (liveStreamCommInfo[streamType][camIndex].streamState == NONE_STREAM_STATE)
        {
            WPRINT(CAMERA_INITIATION, "invld stream state in stop stream: [camera=%d], [stream=%d]", camIndex, streamType);
        }

        MUTEX_LOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
        liveStreamCommInfo[streamType][camIndex].streamState = STOP_STREAM_STATE;
        MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
        DPRINT(CAMERA_INITIATION, "stop request send: [camera=%d], [stream=%d], [state=%d]", camIndex, streamType,
               liveStreamCommInfo[streamType][camIndex].streamState);
        StopCameraStream(camIndex, streamType, FALSE);
    }

    tcpClientInfo[handle].tcpInfo = *tcpInfo;
    tcpClientInfo[handle].tcpHandle = handle;
    tcpClientInfo[handle].tcpResponseCb = (TCP_CALLBACK)callback;

    if (CI_SNP_SHT != tcpInfo->tcpRequest)
    {
        if (FALSE == Utils_CreateThread(NULL, tcpTransfer, &tcpClientInfo[handle], DETACHED_THREAD, TCP_TRANSFER_THREAD_STACK_SZ))
        {
            EPRINT(CAMERA_INITIATION, "fail to create tcp transfer thread");
            MUTEX_LOCK(tcpReqListMutex);
            tcpClientInfo[handle].requestStatus = FREE;
            MUTEX_UNLOCK(tcpReqListMutex);
            return CMD_PROCESS_ERROR;
        }
    }
    else
    {
        if (FALSE == Utils_CreateThread(NULL, tcpTransferImage, &tcpClientInfo[handle], DETACHED_THREAD, IMG_TRANSFER_THREAD_STACK_SZ))
        {
            EPRINT(CAMERA_INITIATION, "fail to create tcp image transfer thread");
            MUTEX_LOCK(tcpReqListMutex);
            tcpClientInfo[handle].requestStatus = FREE;
            MUTEX_UNLOCK(tcpReqListMutex);
            return CMD_PROCESS_ERROR;
        }
    }

    if (NULL != handlePtr)
    {
        *handlePtr = handle;
    }
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function out index of header if success otherwise error
 * @param   tmpBufPtr
 * @param   tempMsgId
 * @return  Message parse success or error reason
 */
static NET_CMD_STATUS_e findHeaderIndexTcpClient(CHARPTR *tmpBufPtr, UINT8PTR tempMsgId)
{
    CHAR buf[CI_MAX_COMM_DATA];

    if (FAIL == ParseStr(tmpBufPtr, FSP, buf, CI_MAX_COMM_DATA))
    {
        EPRINT(CAMERA_INITIATION, "fail to parse tcp client header");
        return CMD_INVALID_SYNTAX;
    }

    *tempMsgId = ConvertStringToIndex(buf, CIReqHeader, MAX_TCP_REQUEST_TYPE);
    if (*tempMsgId >= MAX_TCP_REQUEST_TYPE)
    {
        EPRINT(CAMERA_INITIATION, "invld tcp client request type");
        return CMD_INVALID_MESSAGE;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to get socket buffer size
 * @param   bufOptName
 * @param   socket
 * @return  Size
 */
static UINT32 getBufferSize(INT32 bufOptName, INT32 socket)
{
    UINT32    curSize;
    socklen_t sockOptLen = sizeof(curSize);

    if (getsockopt(socket, SOL_SOCKET, bufOptName, &curSize, &sockOptLen) < STATUS_OK)
    {
        EPRINT(CAMERA_INITIATION, "fail to get buffer size: [err=%s]", STR_ERR);
        return 0;
    }

    return curSize;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used to increase the socket buffer size
 * @param   bufOptName
 * @param   socket
 * @param   requestedSize
 * @return  Size
 */
static UINT32 increaseSocketBufferSize(INT32 bufOptName, INT32 socket, UINT32 requestedSize)
{
    UINT32 curSize = getBufferSize(bufOptName, socket);

    // Next, try to increase the buffer to the requested size, or to some smaller size, if that's not possible:
    if (requestedSize < curSize)
    {
        if (setsockopt(socket, SOL_SOCKET, bufOptName, &requestedSize, sizeof(requestedSize)) >= STATUS_OK)
        {
            // success
            return requestedSize;
        }
    }

    while (requestedSize > curSize)
    {
        if (setsockopt(socket, SOL_SOCKET, bufOptName, &requestedSize, sizeof(requestedSize)) >= STATUS_OK)
        {
            // success
            return requestedSize;
        }

        requestedSize = (requestedSize + curSize) / 2;
    }

    return getBufferSize(bufOptName, socket);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used to increase the socket buffer size
 * @param   camIndex
 * @param   streamType
 */
static void sendStopToCameraInterface(UINT8 camIndex, UINT8 streamType)
{
    TCP_FRAME_INFO_t tcpFrame;

    MUTEX_LOCK(liveStreamCommInfo[streamType][camIndex].tcpCamLock);
    if (INVALID_CONNECTION != liveStreamCommInfo[streamType][camIndex].tcpCommFd)
    {
        CloseCamCmdFd(camIndex, liveStreamCommInfo[streamType][camIndex].tcpCommFdQueueIdx, FALSE);
        liveStreamCommInfo[streamType][camIndex].tcpCommFd = INVALID_CONNECTION;
    }
    MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].tcpCamLock);

    if (NULL != liveStreamCommInfo[streamType][camIndex].tcpFrameCb)
    {
        tcpFrame.state = STOP;
        tcpFrame.dataPtr = NULL;
        tcpFrame.channel = GET_STREAM_MAPPED_CAMERA_ID(camIndex, streamType);
        liveStreamCommInfo[streamType][camIndex].tcpFrameCb(&tcpFrame);
        liveStreamCommInfo[streamType][camIndex].tcpFrameCb = NULL;
    }

    MUTEX_LOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
    liveStreamCommInfo[streamType][camIndex].streamState = NONE_STREAM_STATE;
    MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
    DPRINT(CAMERA_INITIATION, "send stop request to camera interface: [camera=%d], [stream=%d], [state=%d]", camIndex, streamType,
           liveStreamCommInfo[streamType][camIndex].streamState);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It provides status of camera added for live stream. It checks status of all camera and
 *          its main and sub stream status.
 * @param   liveStreamStatusF - Pointer to live stream status of all cameras
 * @return  Returns TRUE if any camera added for live stream else returns FALSE
 */
static BOOL isAnyCameraAddedForLiveStream(BOOL liveStreamStatusF[MAX_STREAM][MAX_CAMERA])
{
    UINT8 camIndex, streamType;

    for (streamType = 0; streamType < MAX_STREAM; streamType++)
    {
        for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
        {
            /* Check live stream status for both streams of camera */
            if (TRUE == liveStreamStatusF[streamType][camIndex])
            {
                /* Camera added for live stream */
                return TRUE;
            }
        }
    }

    /* No camera added for live stream */
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   TCP live stream command handling thread
 * @param   tcpSessionPtr
 * @return
 */
static VOIDPTR tcpLiveStreamCmd(VOIDPTR tcpSessionPtr)
{
    UINT8           camIndex;
    UINT8           streamType;
    UINT8           msgId = MAX_TCP_REQUEST_TYPE;
    CHARPTR         msgPtr = NULL;
    INT32           commFd = INVALID_CONNECTION;
    UINT32          rcvMsgLen;
    UINT8           fdHandle[MAX_STREAM][MAX_CAMERA];
    CAM_CMD_STATE_e camHandleState[MAX_STREAM][MAX_CAMERA];
    CHAR            commMsg[CI_MAX_COMM_DATA];
    UINT64          statusNumber;
    BOOL            liveStreamLocalStatus[MAX_STREAM][MAX_CAMERA];
    UINT64          pollTimeInMs;
    UINT64          currentTimeStamp;
    UINT64          prevTimeStamp[MAX_STREAM][MAX_CAMERA];

    THREAD_START("TCP_STREAM_CMD");

    setpriority(PRIO_PROCESS, PRIO_PROCESS, -1);

    for (streamType = 0; streamType < MAX_STREAM; streamType++)
    {
        for (camIndex = 0; camIndex < MAX_CAMERA; camIndex++)
        {
            fdHandle[streamType][camIndex] = MAX_CMD_QUEUE;
            prevTimeStamp[streamType][camIndex] = 0;
            camHandleState[streamType][camIndex] = CAM_CMD_STATE_INIT;
            liveStreamLocalStatus[streamType][camIndex] = FALSE;
        }
    }

    while (threadExitF == FALSE)
    {
        MUTEX_LOCK(liveStreamInfo[CMD_STREAM].mutex);
        if (FALSE == isAnyCameraAddedForLiveStream(liveStreamInfo[CMD_STREAM].status))
        {
            pthread_cond_wait(&liveStreamInfo[CMD_STREAM].signal, &liveStreamInfo[CMD_STREAM].mutex);
        }
        memcpy(liveStreamLocalStatus, liveStreamInfo[CMD_STREAM].status, sizeof(liveStreamLocalStatus));
        MUTEX_UNLOCK(liveStreamInfo[CMD_STREAM].mutex);

        for (streamType = 0; streamType < MAX_STREAM; streamType++)
        {
            for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
            {
                if (FALSE == liveStreamLocalStatus[streamType][camIndex])
                {
                    continue;
                }

                MUTEX_LOCK(liveStreamInfo[LIVE_STREAM].mutex);
                if (TRUE == liveStreamInfo[LIVE_STREAM].status[streamType][camIndex])
                {
                    liveStreamInfo[LIVE_STREAM].status[streamType][camIndex] = FALSE;
                    pthread_cond_signal(&liveStreamInfo[LIVE_STREAM].signal);
                }
                MUTEX_UNLOCK(liveStreamInfo[LIVE_STREAM].mutex);

                if (CAM_CMD_STATE_INIT != camHandleState[streamType][camIndex])
                {
                    continue;
                }

                if (FAIL == AddCmdCamCount(camIndex, &fdHandle[streamType][camIndex]))
                {
                    CI_RESET_LIVE_STREAM_STS(CMD_STREAM, camIndex, streamType);
                    sendStopToCameraInterface(camIndex, streamType);
                    fdHandle[streamType][camIndex] = MAX_CMD_QUEUE;
                    camHandleState[streamType][camIndex] = CAM_CMD_STATE_INIT;
                    prevTimeStamp[streamType][camIndex] = 0;
                    EPRINT(CAMERA_INITIATION, "fail to get ci camera fd handle: [camera=%d], [stream=%d]", camIndex, streamType);
                    continue;
                }

                camHandleState[streamType][camIndex] = CAM_CMD_STATE_HANDLE_GOT;
                DPRINT(CAMERA_INITIATION, "got ci fd handle: [camera=%d], [stream=%d]", camIndex, streamType);
            }
        }

        /* Get CI polling time in seconds and convert into milli seconds */
        pollTimeInMs = ((UINT64)GetCiPollTimeInSec()) * MILLI_SEC_PER_SEC;
        for (streamType = 0; streamType < MAX_STREAM; streamType++)
        {
            for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
            {
                if ((camHandleState[streamType][camIndex] != CAM_CMD_STATE_HANDLE_GOT) &&
                    (camHandleState[streamType][camIndex] != CAM_CMD_STATE_FD_WAIT))
                {
                    continue;
                }

                if (FALSE == liveStreamLocalStatus[streamType][camIndex])
                {
                    continue;
                }

                currentTimeStamp = GetMonotonicTimeInMilliSec();

                MUTEX_LOCK(liveStreamInfo[LIVE_STREAM].mutex);
                if (TRUE == liveStreamInfo[LIVE_STREAM].status[streamType][camIndex])
                {
                    liveStreamInfo[LIVE_STREAM].status[streamType][camIndex] = FALSE;
                    pthread_cond_signal(&liveStreamInfo[LIVE_STREAM].signal);
                }
                MUTEX_UNLOCK(liveStreamInfo[LIVE_STREAM].mutex);

                /* Provide sleep to reduce CPU usage */
                usleep(1000);

                if (FAIL == GetCmdFd(camIndex, &commFd, &fdHandle[streamType][camIndex], FALSE))
                {
                    camHandleState[streamType][camIndex] = CAM_CMD_STATE_FD_WAIT;
                    if (prevTimeStamp[streamType][camIndex] == 0)
                    {
                        /* Store current timestamp as benchmark for future comparision */
                        prevTimeStamp[streamType][camIndex] = currentTimeStamp;
                        continue;
                    }

                    if (pollTimeInMs > (currentTimeStamp - prevTimeStamp[streamType][camIndex]))
                    {
                        /* Timeout is not occurred yet */
                        continue;
                    }

                    EPRINT(CAMERA_INITIATION, "live stream get fd timeout: [camera=%d], [stream=%d], [handle=%d]", camIndex, streamType,
                           fdHandle[streamType][camIndex]);
                    prevTimeStamp[streamType][camIndex] = 0;
                    CI_RESET_LIVE_STREAM_STS(CMD_STREAM, camIndex, streamType);
                    camHandleState[streamType][camIndex] = CAM_CMD_STATE_INIT;
                    sendStopToCameraInterface(camIndex, streamType);
                    GetCmdFd(camIndex, &commFd, &fdHandle[streamType][camIndex], TRUE);
                    fdHandle[streamType][camIndex] = MAX_CMD_QUEUE;
                    continue;
                }

                prevTimeStamp[streamType][camIndex] = 0;
                camHandleState[streamType][camIndex] = CAM_CMD_STATE_FD_GOT;
                DPRINT(CAMERA_INITIATION, "got fd for streaming: [camera=%d], [stream=%d]", camIndex, streamType);

                MUTEX_LOCK(liveStreamCommInfo[streamType][camIndex].tcpCamLock);
                if (INVALID_CONNECTION != liveStreamCommInfo[streamType][camIndex].tcpCommFd)
                {
                    CloseCamCmdFd(camIndex, liveStreamCommInfo[streamType][camIndex].tcpCommFdQueueIdx, FALSE);
                    liveStreamCommInfo[streamType][camIndex].tcpCommFd = INVALID_CONNECTION;
                }

                liveStreamCommInfo[streamType][camIndex].tcpCommFd = commFd;
                liveStreamCommInfo[streamType][camIndex].tcpCommFdQueueIdx = fdHandle[streamType][camIndex];
                snprintf(commMsg, CI_MAX_COMM_DATA, "%c%s%c%s%c%d%c%c", SOM, CIReqHeader[CI_SET_CMD], FSP, CIReqHeader[CI_SRT_LV_STRM], FSP,
                         liveStreamCommInfo[streamType][camIndex].profileNum, FSP, EOM);
                MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].tcpCamLock);

                DPRINT(CAMERA_INITIATION, "start stream cmd msg: [camera=%d], [stream=%d], [msg=%s]", camIndex, streamType, commMsg);
                if (FAIL == SendToSocket(commFd, (UINT8PTR)commMsg, strlen(commMsg), CI_LV_CMD_SND_RCV))
                {
                    EPRINT(CAMERA_INITIATION, "fail to send start stream request: [camera=%d], [stream=%d]", camIndex, streamType);
                    CI_RESET_LIVE_STREAM_STS(CMD_STREAM, camIndex, streamType);
                    sendStopToCameraInterface(camIndex, streamType);
                    fdHandle[streamType][camIndex] = MAX_CMD_QUEUE;
                    camHandleState[streamType][camIndex] = CAM_CMD_STATE_INIT;
                    continue;
                }

                if (SUCCESS != RecvMessage(commFd, commMsg, &rcvMsgLen, SOM, EOM, CI_MAX_COMM_DATA - 1, CI_LV_CMD_SND_RCV))
                {
                    EPRINT(CAMERA_INITIATION, "fail to recv start stream response: [camera=%d], [stream=%d]", camIndex, streamType);
                    CI_RESET_LIVE_STREAM_STS(CMD_STREAM, camIndex, streamType);
                    fdHandle[streamType][camIndex] = MAX_CMD_QUEUE;
                    camHandleState[streamType][camIndex] = CAM_CMD_STATE_INIT;
                    sendStopToCameraInterface(camIndex, streamType);
                    continue;
                }

                commMsg[rcvMsgLen] = '\0';
                msgPtr = commMsg + 1;

                if (CMD_SUCCESS != findHeaderIndexTcpClient(&msgPtr, &msgId))
                {
                    EPRINT(CAMERA_INITIATION, "fail to parse msg id: [camera=%d], [stream=%d]", camIndex, streamType);
                    CI_RESET_LIVE_STREAM_STS(CMD_STREAM, camIndex, streamType);
                    sendStopToCameraInterface(camIndex, streamType);
                    fdHandle[streamType][camIndex] = MAX_CMD_QUEUE;
                    camHandleState[streamType][camIndex] = CAM_CMD_STATE_INIT;
                    continue;
                }

                switch (msgId)
                {
                    case CI_RPL_CMD:
                    {
                        if (SUCCESS != ParseStringGetVal(&msgPtr, &statusNumber, 1, FSP))
                        {
                            sendStopToCameraInterface(camIndex, streamType);
                            break;
                        }

                        if (CI_CMD_SUCCESS != statusNumber)
                        {
                            EPRINT(CAMERA_INITIATION, "start stream cmd fail: [camera=%d], [stream=%d], [status=%lld]", camIndex, streamType,
                                   statusNumber);
                            sendStopToCameraInterface(camIndex, streamType);
                            fdHandle[streamType][camIndex] = MAX_CMD_QUEUE;
                            camHandleState[streamType][camIndex] = CAM_CMD_STATE_INIT;
                            break;
                        }

                        MUTEX_LOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
                        switch (liveStreamCommInfo[streamType][camIndex].streamState)
                        {
                            case CMD_STREAM_STATE:
                            {
                                liveStreamCommInfo[streamType][camIndex].streamState = LIVE_STREAM_STATE;
                                MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
                                DPRINT(CAMERA_INITIATION, "cmd request send: [camera=%d], [stream=%d], [state=%d]", camIndex, streamType,
                                       liveStreamCommInfo[streamType][camIndex].streamState);
                            }
                            break;

                            case NONE_STREAM_STATE:
                            {
                                MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
                                fdHandle[streamType][camIndex] = MAX_CMD_QUEUE;
                                continue;
                            }
                            break;

                            default:
                            {
                                MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
                                StopCameraStream(camIndex, streamType, TRUE);
                                EPRINT(CAMERA_INITIATION, "cmd request send: [camera=%d], [stream=%d], [state=%d]", camIndex, streamType,
                                       liveStreamCommInfo[streamType][camIndex].streamState);
                                fdHandle[streamType][camIndex] = MAX_CMD_QUEUE;
                                continue;
                            }
                            break;
                        }

                        MUTEX_LOCK(liveStreamInfo[LIVE_STREAM].mutex);
                        if (TRUE == liveStreamInfo[LIVE_STREAM].status[streamType][camIndex])
                        {
                            /* Streaming is already going on */
                            MUTEX_UNLOCK(liveStreamInfo[LIVE_STREAM].mutex);
                            EPRINT(CAMERA_INITIATION, "streaming is already going on: [camera=%d], [stream=%d]", camIndex, streamType);
                            sendStopToCameraInterface(camIndex, streamType);
                        }
                        else
                        {
                            liveStreamInfo[LIVE_STREAM].status[streamType][camIndex] = TRUE;
                            pthread_cond_signal(&liveStreamInfo[LIVE_STREAM].signal);
                            MUTEX_UNLOCK(liveStreamInfo[LIVE_STREAM].mutex);

                            DPRINT(CAMERA_INITIATION, "increased socket buffer size: [camera=%d], [stream=%d], [send=%d], [recv=%d]", camIndex,
                                   streamType, increaseSocketBufferSize(SO_SNDBUF, commFd, MAX_SEND_LEN_SOCK),
                                   increaseSocketBufferSize(SO_RCVBUF, commFd, MAX_RVC_LEN_SOCK));
                        }
                    }
                    break;

                    default:
                    {
                        sendStopToCameraInterface(camIndex, streamType);
                    }
                    break;
                }

                fdHandle[streamType][camIndex] = MAX_CMD_QUEUE;
                CI_RESET_LIVE_STREAM_STS(CMD_STREAM, camIndex, streamType);
                camHandleState[streamType][camIndex] = CAM_CMD_STATE_INIT;
            }
        }
    }

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Reset tcp live stream frame data information
 * @param   pFrameInfo
 */
static void resetTcpLiveStreamFrameInfo(FRAME_DATA_INFO_t *pFrameInfo)
{
    pFrameInfo->frameRecvF = FALSE;
    pFrameInfo->timeoutCnt = 0;
    pFrameInfo->frameState = FRAME_RECV_STATE_FIRST_FRAME;
    pFrameInfo->headerRef = NULL;
    pFrameInfo->payloadRef = NULL;
    pFrameInfo->payloadLen = 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   TCP live streaming thread
 * @param   tcpSessionPtr
 * @return
 */
static VOIDPTR tcpLiveStream(VOIDPTR tcpSessionPtr)
{
    BOOL              status;
    UINT8             camIndex;
    UINT8             streamType;
    UINT32            recvMsgLen;
    INT32             streamFd;
    UINT32            loopCnt;
    UINT32            dataLen;
    UINT32            dataRecvLen;
    UINT32            maxFrameLen;
    TCP_FRAME_INFO_t  tcpFrame;
    FRAME_HEADER_t   *pMediaFrameHeader = NULL;
    BOOL              liveStreamLocalStatus[MAX_STREAM][MAX_CAMERA];
    BOOL              newLiveStreamLocalStatus[MAX_STREAM][MAX_CAMERA];
    FRAME_DATA_INFO_t frameInfo[MAX_STREAM][MAX_CAMERA];

    THREAD_START("TCP_LIVE_STREAM");

    setpriority(PRIO_PROCESS, PRIO_PROCESS, -1);

    for (streamType = 0; streamType < MAX_STREAM; streamType++)
    {
        for (camIndex = 0; camIndex < MAX_CAMERA; camIndex++)
        {
            liveStreamLocalStatus[streamType][camIndex] = FALSE;
            newLiveStreamLocalStatus[streamType][camIndex] = FALSE;
            frameInfo[streamType][camIndex].payloadData = NULL;
            resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);
        }
    }

    while (threadExitF == FALSE)
    {
        MUTEX_LOCK(liveStreamInfo[LIVE_STREAM].mutex);
        if ((memcmp(liveStreamInfo[LIVE_STREAM].status, liveStreamLocalStatus, sizeof(liveStreamLocalStatus)) == 0) &&
            (FALSE == isAnyCameraAddedForLiveStream(liveStreamInfo[LIVE_STREAM].status)))
        {
            DPRINT(CAMERA_INITIATION, "tcp live stream thread waiting");
            pthread_cond_wait(&liveStreamInfo[LIVE_STREAM].signal, &liveStreamInfo[LIVE_STREAM].mutex);
            DPRINT(CAMERA_INITIATION, "tcp live stream thread waked-up");
        }
        memcpy(newLiveStreamLocalStatus, liveStreamInfo[LIVE_STREAM].status, sizeof(newLiveStreamLocalStatus));
        MUTEX_UNLOCK(liveStreamInfo[LIVE_STREAM].mutex);

        for (streamType = 0; streamType < MAX_STREAM; streamType++)
        {
            for (camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
            {
                if (liveStreamLocalStatus[streamType][camIndex] != newLiveStreamLocalStatus[streamType][camIndex])
                {
                    /* Update the camera status to old variable */
                    liveStreamLocalStatus[streamType][camIndex] = newLiveStreamLocalStatus[streamType][camIndex];

                    FREE_MEMORY(frameInfo[streamType][camIndex].payloadData);
                    resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);

                    if ((FALSE == newLiveStreamLocalStatus[streamType][camIndex]) || (INACTIVE == GetCameraConnectionStatus(camIndex)))
                    {
                        status = FAIL;
                        MUTEX_LOCK(liveStreamCommInfo[streamType][camIndex].tcpCamLock);
                        if (INVALID_CONNECTION != liveStreamCommInfo[streamType][camIndex].tcpCommFd)
                        {
                            EPRINT(CAMERA_INITIATION, "stop reached for camera: [camera=%d], [stream=%d]", camIndex, streamType);
                            CloseCamCmdFd(camIndex, liveStreamCommInfo[streamType][camIndex].tcpCommFdQueueIdx, FALSE);
                            liveStreamCommInfo[streamType][camIndex].tcpCommFd = INVALID_CONNECTION;
                            liveStreamCommInfo[streamType][camIndex].tcpCommFdQueueIdx = MAX_CMD_QUEUE;
                            status = SUCCESS;
                        }
                        MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].tcpCamLock);

                        if ((SUCCESS == status) && (liveStreamCommInfo[streamType][camIndex].tcpFrameCb != NULL))
                        {
                            EPRINT(CAMERA_INITIATION, "send stop to camera interface: [camera=%d], [stream=%d]", camIndex, streamType);
                            tcpFrame.state = STOP;
                            tcpFrame.dataPtr = NULL;
                            tcpFrame.channel = GET_STREAM_MAPPED_CAMERA_ID(camIndex, streamType);
                            liveStreamCommInfo[streamType][camIndex].tcpFrameCb(&tcpFrame);
                            liveStreamCommInfo[streamType][camIndex].tcpFrameCb = NULL;
                        }

                        MUTEX_LOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
                        liveStreamCommInfo[streamType][camIndex].streamState = NONE_STREAM_STATE;
                        MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
                        DPRINT(CAMERA_INITIATION, "request send: [camera=%d], [stream=%d], [state=%d]", camIndex, streamType,
                               liveStreamCommInfo[streamType][camIndex].streamState);
                        continue;
                    }

                    MUTEX_LOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
                    switch (liveStreamCommInfo[streamType][camIndex].streamState)
                    {
                        case LIVE_STREAM_STATE:
                        {
                            liveStreamCommInfo[streamType][camIndex].streamState = RUN_STREAM_STATE;
                            MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
                            DPRINT(CAMERA_INITIATION, "live stream request send: [camera=%d], [stream=%d], [state=%d]", camIndex, streamType,
                                   liveStreamCommInfo[streamType][camIndex].streamState);
                            status = SUCCESS;
                        }
                        break;

                        case NONE_STREAM_STATE:
                        {
                            MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
                            status = FAIL;
                        }
                        break;

                        default:
                        {
                            MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].streamMutex);
                            CI_RESET_LIVE_STREAM_STS(LIVE_STREAM, camIndex, streamType);
                            status = FAIL;
                        }
                        break;
                    }

                    if (FAIL == status)
                    {
                        continue;
                    }
                }
                else
                {
                    if (FALSE == newLiveStreamLocalStatus[streamType][camIndex])
                    {
                        continue;
                    }
                }

                MUTEX_LOCK(liveStreamCommInfo[streamType][camIndex].tcpCamLock);
                streamFd = liveStreamCommInfo[streamType][camIndex].tcpCommFd;
                MUTEX_UNLOCK(liveStreamCommInfo[streamType][camIndex].tcpCamLock);

                if ((INVALID_CONNECTION == streamFd) || (INACTIVE == GetCameraConnectionStatus(camIndex)))
                {
                    EPRINT(CAMERA_INITIATION, "camera inactive or invld fd, so stop live stream: [camera=%d], [stream=%d], [fd=%d]", camIndex,
                           streamType, streamFd);
                    CI_RESET_LIVE_STREAM_STS(LIVE_STREAM, camIndex, streamType);
                    resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);
                    continue;
                }

                if (NULL == frameInfo[streamType][camIndex].payloadData)
                {
                    frameInfo[streamType][camIndex].payloadData = (UINT8PTR)malloc(CI_MAX_FRAME_LEN);
                    if (NULL == frameInfo[streamType][camIndex].payloadData)
                    {
                        EPRINT(CAMERA_INITIATION, "fail to allocate memory: [camera=%d], [stream=%d]", camIndex, streamType);
                        CI_RESET_LIVE_STREAM_STS(LIVE_STREAM, camIndex, streamType);
                        resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);
                        continue;
                    }
                }

                /* Initialize the local variables */
                recvMsgLen = dataRecvLen = loopCnt = 0;
                maxFrameLen = CI_MAX_FRAME_LEN;
                pMediaFrameHeader = NULL;

                /* Update frame data ptr only if null */
                if (NULL == frameInfo[streamType][camIndex].payloadRef)
                {
                    frameInfo[streamType][camIndex].payloadRef = frameInfo[streamType][camIndex].payloadData;
                    frameInfo[streamType][camIndex].headerRef = frameInfo[streamType][camIndex].headerData;
                }

                do
                {
                    if (loopCnt >= MAX_LOOP_ALLOW)
                    {
                        break;
                    }

                    switch (frameInfo[streamType][camIndex].frameState)
                    {
                        case FRAME_RECV_STATE_FIRST_FRAME:
                        {
                            /* if I-Frame Recv then now recv the 40 byte header */
                            if (TRUE == frameInfo[streamType][camIndex].frameRecvF)
                            {
                                dataRecvLen = CI_FRAME_HEADER_LEN;
                            }
                            else
                            {
                                /* If I-Frame not Recv till then find the magic code */
                                if (0 == dataRecvLen)
                                {
                                    dataRecvLen = MAX_MAGIC_CODE_LEN;
                                }
                            }
                        }
                        break;

                        case FRAME_RECV_STATE_MAGIC_CODE:
                        {
                            dataRecvLen = CI_FRAME_HEADER_LEN - MAX_MAGIC_CODE_LEN;
                        }
                        break;

                        case FRAME_RECV_STATE_LEFT_HEADER:
                        {
                            if (frameInfo[streamType][camIndex].headerRef == NULL)
                            {
                                break;
                            }

                            if (frameInfo[streamType][camIndex].headerRef <= frameInfo[streamType][camIndex].headerData)
                            {
                                break;
                            }

                            dataLen = frameInfo[streamType][camIndex].headerRef - frameInfo[streamType][camIndex].headerData;
                            if (dataLen > CI_FRAME_HEADER_LEN)
                            {
                                dataRecvLen = 0;
                                EPRINT(CAMERA_INITIATION, "invld frame length found: [camera=%d], [stream=%d]", camIndex, streamType);
                                resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);
                                break;
                            }

                            if (CI_FRAME_HEADER_LEN == dataLen)
                            {
                                dataRecvLen = 0;
                                recvMsgLen = CI_FRAME_HEADER_LEN;
                                frameInfo[streamType][camIndex].frameState = FRAME_RECV_STATE_NEW_HEADER;
                                memcpy(frameInfo[streamType][camIndex].payloadData, frameInfo[streamType][camIndex].headerData, CI_FRAME_HEADER_LEN);
                                frameInfo[streamType][camIndex].payloadRef = frameInfo[streamType][camIndex].payloadData;
                                frameInfo[streamType][camIndex].headerRef = NULL;
                                continue;
                            }

                            /* Receive remaining frame header */
                            dataRecvLen = CI_FRAME_HEADER_LEN - dataLen;
                        }
                        break;

                        case FRAME_RECV_STATE_LEFT_PAYLOAD:
                        {
                            pMediaFrameHeader = (FRAME_HEADER_t *)frameInfo[streamType][camIndex].headerData;
                            dataRecvLen = (UINT32)(pMediaFrameHeader->mediaFrmLen);
                        }
                        break;

                        case FRAME_RECV_STATE_NEW_HEADER:
                        {
                            pMediaFrameHeader = (FRAME_HEADER_t *)frameInfo[streamType][camIndex].headerData;
                            if (pMediaFrameHeader->mediaFrmLen <= CI_FRAME_HEADER_LEN)
                            {
                                dataRecvLen = 0;
                                EPRINT(CAMERA_INITIATION, "invld frame length found: [camera=%d], [stream=%d]", camIndex, streamType);
                                resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);
                                break;
                            }

                            pMediaFrameHeader->mediaFrmLen -= CI_FRAME_HEADER_LEN;
                            tcpFrame.frameInfo.len = (UINT32)(pMediaFrameHeader->mediaFrmLen);
                            dataRecvLen = tcpFrame.frameInfo.len;
                        }
                        break;

                        default:
                        {
                            /* Nothing to do */
                        }
                        break;
                    }

                    /* If length is 0, start from beginning */
                    if (dataRecvLen == 0)
                    {
                        EPRINT(CAMERA_INITIATION, "invld operation: [camera=%d], [stream=%d], [camState=%d]", camIndex, streamType,
                               frameInfo[streamType][camIndex].frameState);
                        break;
                    }

                    /* Receive frame from socket */
                    loopCnt++;
                    status = RecvFrame(streamFd, (CHARPTR)frameInfo[streamType][camIndex].payloadRef, &recvMsgLen, dataRecvLen, 0, 0);
                    if (REFUSE == status)
                    {
                        EPRINT(CAMERA_INITIATION, "camera connection closed: [camera=%d], [stream=%d]", camIndex, streamType);
                        CI_RESET_LIVE_STREAM_STS(LIVE_STREAM, camIndex, streamType);
                        resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);
                        break;
                    }

                    if ((FAIL == status) || (FRAME_RECV_STATE_LEFT_HEADER == frameInfo[streamType][camIndex].frameState) ||
                        (FRAME_RECV_STATE_LEFT_PAYLOAD == frameInfo[streamType][camIndex].frameState))
                    {
                        if (0 == recvMsgLen)
                        {
                            if (FRAME_RECV_STATE_NEW_HEADER == frameInfo[streamType][camIndex].frameState)
                            {
                                frameInfo[streamType][camIndex].frameState = FRAME_RECV_STATE_LEFT_PAYLOAD;
                            }

                            frameInfo[streamType][camIndex].timeoutCnt++;
                            if (frameInfo[streamType][camIndex].timeoutCnt >= MAX_TIMEOUT_STREAM)
                            {
                                EPRINT(CAMERA_INITIATION, "timeout cnt reached max: [camera=%d], [stream=%d]", camIndex, streamType);
                                CI_RESET_LIVE_STREAM_STS(LIVE_STREAM, camIndex, streamType);
                                resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);
                            }
                            break;
                        }

                        if (recvMsgLen <= dataRecvLen)
                        {
                            if ((FRAME_RECV_STATE_LEFT_HEADER == frameInfo[streamType][camIndex].frameState) ||
                                (FRAME_RECV_STATE_FIRST_FRAME == frameInfo[streamType][camIndex].frameState) ||
                                (FRAME_RECV_STATE_MAGIC_CODE == frameInfo[streamType][camIndex].frameState))
                            {
                                if ((FRAME_RECV_STATE_FIRST_FRAME == frameInfo[streamType][camIndex].frameState) ||
                                    (FRAME_RECV_STATE_MAGIC_CODE == frameInfo[streamType][camIndex].frameState))
                                {
                                    frameInfo[streamType][camIndex].headerRef = frameInfo[streamType][camIndex].headerData;
                                    frameInfo[streamType][camIndex].frameState = FRAME_RECV_STATE_LEFT_HEADER;
                                }

                                if (frameInfo[streamType][camIndex].payloadRef > frameInfo[streamType][camIndex].payloadData)
                                {
                                    dataLen = frameInfo[streamType][camIndex].payloadRef - frameInfo[streamType][camIndex].payloadData;
                                    EPRINT(CAMERA_INITIATION, "invld frame length found: [camera=%d], [stream=%d], [dataLen=%d]", camIndex,
                                           streamType, dataLen);
                                    if (dataLen > CI_FRAME_HEADER_LEN)
                                    {
                                        resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);
                                        break;
                                    }

                                    memcpy(frameInfo[streamType][camIndex].headerRef, frameInfo[streamType][camIndex].payloadData, dataLen);
                                    frameInfo[streamType][camIndex].headerRef += dataLen;
                                }

                                memcpy(frameInfo[streamType][camIndex].headerRef, frameInfo[streamType][camIndex].payloadRef, recvMsgLen);
                                frameInfo[streamType][camIndex].headerRef += recvMsgLen;

                                if (CI_FRAME_HEADER_LEN != (frameInfo[streamType][camIndex].headerRef - frameInfo[streamType][camIndex].headerData))
                                {
                                    /* If header is left then come again to receive left header */
                                    break;
                                }

                                frameInfo[streamType][camIndex].frameState = FRAME_RECV_STATE_MAGIC_CODE;
                                memcpy(frameInfo[streamType][camIndex].payloadData, frameInfo[streamType][camIndex].headerData, CI_FRAME_HEADER_LEN);
                                pMediaFrameHeader = (FRAME_HEADER_t *)frameInfo[streamType][camIndex].headerData;
                                frameInfo[streamType][camIndex].payloadLen = pMediaFrameHeader->mediaFrmLen - CI_FRAME_HEADER_LEN;
                                recvMsgLen = CI_FRAME_HEADER_LEN;
                                frameInfo[streamType][camIndex].payloadRef = frameInfo[streamType][camIndex].payloadData;
                                frameInfo[streamType][camIndex].headerRef = NULL;
                            }
                            else if ((FRAME_RECV_STATE_NEW_HEADER == frameInfo[streamType][camIndex].frameState) ||
                                     (FRAME_RECV_STATE_LEFT_PAYLOAD == frameInfo[streamType][camIndex].frameState))
                            {
                                pMediaFrameHeader = (FRAME_HEADER_t *)frameInfo[streamType][camIndex].headerData;
                                frameInfo[streamType][camIndex].frameState = FRAME_RECV_STATE_LEFT_PAYLOAD;

                                if (pMediaFrameHeader->mediaFrmLen >= recvMsgLen)
                                {
                                    pMediaFrameHeader->mediaFrmLen -= ((UINT32)recvMsgLen);
                                    frameInfo[streamType][camIndex].payloadRef += recvMsgLen;
                                    maxFrameLen -= MAX_MAGIC_CODE_LEN;
                                }
                                else
                                {
                                    pMediaFrameHeader->mediaFrmLen = 0;
                                }

                                if (pMediaFrameHeader->mediaFrmLen > 0)
                                {
                                    break;
                                }

                                frameInfo[streamType][camIndex].frameState = FRAME_RECV_STATE_NEW_HEADER;
                            }
                        }
                    }
                    else
                    {
                        /* Reset the time out count to receive frame */
                        frameInfo[streamType][camIndex].timeoutCnt = 0;
                    }

                    /* Just a double check */
                    if (recvMsgLen == 0)
                    {
                        DPRINT(CAMERA_INITIATION, "no data rcvd: [camera=%d], [stream=%d]", camIndex, streamType);
                        resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);
                        break;
                    }

                    if ((FRAME_RECV_STATE_FIRST_FRAME == frameInfo[streamType][camIndex].frameState) ||
                        (FRAME_RECV_STATE_MAGIC_CODE == frameInfo[streamType][camIndex].frameState))
                    {
                        if ((MAX_MAGIC_CODE_LEN == recvMsgLen) && (FALSE == frameInfo[streamType][camIndex].frameRecvF))
                        {
                            pMediaFrameHeader = (FRAME_HEADER_t *)frameInfo[streamType][camIndex].payloadRef;
                            if (pMediaFrameHeader->magicCode == MAGIC_CODE)
                            {
                                /* Again read the frame after the magic code */
                                DPRINT(CAMERA_INITIATION, "magic code found: [camera=%d], [stream=%d]", camIndex, streamType);
                                frameInfo[streamType][camIndex].frameState = FRAME_RECV_STATE_MAGIC_CODE;
                                frameInfo[streamType][camIndex].payloadRef += MAX_MAGIC_CODE_LEN;
                                maxFrameLen -= MAX_MAGIC_CODE_LEN;
                            }
                            else
                            {
                                dataRecvLen = 0;
                                frameInfo[streamType][camIndex].frameState = FRAME_RECV_STATE_FIRST_FRAME;
                            }
                            continue;
                        }

                        dataRecvLen = 0;
                        pMediaFrameHeader = (FRAME_HEADER_t *)frameInfo[streamType][camIndex].payloadData;
                        if ((MAGIC_CODE != pMediaFrameHeader->magicCode) || (CAM_PRODUCT_TYPE != pMediaFrameHeader->productType))
                        {
                            EPRINT(CAMERA_INITIATION,
                                   "invld magic code or product type found: [camera=%d], [stream=%d], [magicCode=0x%x], [productType=%d]", camIndex,
                                   streamType, pMediaFrameHeader->magicCode, pMediaFrameHeader->productType);
                            resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);
                            break;
                        }

                        if ((FALSE == frameInfo[streamType][camIndex].frameRecvF) && (I_FRAME == pMediaFrameHeader->frmType))
                        {
                            frameInfo[streamType][camIndex].frameRecvF = TRUE;
                        }

                        if (recvMsgLen <= CI_FRAME_HEADER_LEN)
                        {
                            /* Receive frame header */
                            frameInfo[streamType][camIndex].frameState = FRAME_RECV_STATE_NEW_HEADER;
                            memcpy(frameInfo[streamType][camIndex].headerData, frameInfo[streamType][camIndex].payloadData, CI_FRAME_HEADER_LEN);
                            pMediaFrameHeader = (FRAME_HEADER_t *)frameInfo[streamType][camIndex].headerData;
                            frameInfo[streamType][camIndex].payloadLen = pMediaFrameHeader->mediaFrmLen - CI_FRAME_HEADER_LEN;

                            if (frameInfo[streamType][camIndex].payloadLen >= CI_MAX_FRAME_LEN)
                            {
                                EPRINT(CAMERA_INITIATION, "frame is greater size. update buffer memory: [camera=%d], [stream=%d]", camIndex,
                                       streamType);
                                resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);
                                break;
                            }

                            frameInfo[streamType][camIndex].payloadRef += recvMsgLen;
                            maxFrameLen -= recvMsgLen;
                            continue;
                        }
                    }
                    else if ((FRAME_RECV_STATE_NEW_HEADER == frameInfo[streamType][camIndex].frameState) ||
                             (FRAME_RECV_STATE_LEFT_PAYLOAD == frameInfo[streamType][camIndex].frameState))
                    {
                        loopCnt = 0;
                        if (FALSE == frameInfo[streamType][camIndex].frameRecvF)
                        {
                            if (FRAME_RECV_STATE_LEFT_PAYLOAD == frameInfo[streamType][camIndex].frameState)
                            {
                                continue;
                            }

                            /* Note: If I-Frame Not Recv until then ignore all the frames */
                            resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);
                            break;
                        }

                        if (FRAME_RECV_STATE_NEW_HEADER == frameInfo[streamType][camIndex].frameState)
                        {
                            frameInfo[streamType][camIndex].frameState = FRAME_RECV_STATE_NEW_PAYLOAD;
                        }

                        pMediaFrameHeader = (FRAME_HEADER_t *)frameInfo[streamType][camIndex].headerData;
                        if (MAGIC_CODE != (UINT32)pMediaFrameHeader->magicCode)
                        {
                            EPRINT(CAMERA_INITIATION, "header not found, reseting logic: [camera=%d], [stream=%d]", camIndex, streamType);
                            resetTcpLiveStreamFrameInfo(&frameInfo[streamType][camIndex]);
                            break;
                        }

                        if (pMediaFrameHeader->vidResolution >= MAX_CI_RESOLUTION)
                        {
                            EPRINT(CAMERA_INITIATION, "invld frame resolution: [camera=%d], [stream=%d], [resolution=%d]", camIndex, streamType,
                                   pMediaFrameHeader->vidResolution);
                            break;
                        }

                        if ((pMediaFrameHeader->frmType != I_FRAME) && (pMediaFrameHeader->frmType != P_FRAME))
                        {
                            EPRINT(CAMERA_INITIATION, "different frame type found: [camera=%d], [stream=%d], [frameType=%d]", camIndex, streamType,
                                   pMediaFrameHeader->frmType);
                            break;
                        }

                        if (NULL == liveStreamCommInfo[streamType][camIndex].tcpFrameCb)
                        {
                            EPRINT(CAMERA_INITIATION, "frame callback is not set: [camera=%d], [stream=%d]", camIndex, streamType);
                            break;
                        }

                        tcpFrame.state = START;
                        tcpFrame.channel = GET_STREAM_MAPPED_CAMERA_ID(camIndex, streamType);
                        tcpFrame.streamType = pMediaFrameHeader->streamType;
                        tcpFrame.frameInfo.isRTSP = TRUE;
                        tcpFrame.frameInfo.avPresentationTime.tv_sec = pMediaFrameHeader->localTime.totalSec;
                        tcpFrame.frameInfo.avPresentationTime.tv_usec = (pMediaFrameHeader->localTime.mSec * 1000L);
                        tcpFrame.frameInfo.len = frameInfo[streamType][camIndex].payloadLen;
                        tcpFrame.frameInfo.codecType = pMediaFrameHeader->codecType;
                        tcpFrame.frameInfo.sampleRate = pMediaFrameHeader->audSampleFrq;
                        tcpFrame.frameInfo.videoInfo.frameType = pMediaFrameHeader->frmType;
                        tcpFrame.frameInfo.videoInfo.noOfRefFrame = DEFAULT_REF_NUM; /* We take default reference frame */

                        if (pMediaFrameHeader->vidResolution == 0)
                        {
                            tcpFrame.frameInfo.videoInfo.width = INVALID_HEIGHT_WIDTH;
                            tcpFrame.frameInfo.videoInfo.height = INVALID_HEIGHT_WIDTH;
                        }
                        else
                        {
                            tcpFrame.frameInfo.videoInfo.width = frameResolution[pMediaFrameHeader->vidResolution][FRAME_WIDTH];
                            tcpFrame.frameInfo.videoInfo.height = frameResolution[pMediaFrameHeader->vidResolution][FRAME_HEIGHT];
                        }

                        /* Provide frame to camera interface for further processing */
                        tcpFrame.dataPtr = frameInfo[streamType][camIndex].payloadData + CI_FRAME_HEADER_LEN;
                        liveStreamCommInfo[streamType][camIndex].tcpFrameCb(&tcpFrame);
                        frameInfo[streamType][camIndex].payloadRef = NULL;

                        if (FRAME_RECV_STATE_NEW_PAYLOAD == frameInfo[streamType][camIndex].frameState)
                        {
                            frameInfo[streamType][camIndex].frameState = FRAME_RECV_STATE_FIRST_FRAME;
                        }
                        break;
                    }

                } while (TRUE);
            }
        }

        /* Provide sleep to reduce CPU usage */
        usleep(1000);
    }

    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a thread function. It performs actual communication with server
 * @param   tcpSessionPtr
 * @return
 */
static VOIDPTR tcpTransfer(VOIDPTR tcpSessionPtr)
{
    TCP_CLIENT_INFO_t *publicTcpInfo = (TCP_CLIENT_INFO_t *)tcpSessionPtr;
    TCP_RESPONSE_e     tcpResp = TCP_SUCCESS;
    CHAR               commMsg[CI_MAX_COMM_DATA];
    UINT8              camIndex = MAX_CAMERA, realCamIndex = MAX_CAMERA;
    UINT32             timeOut = 0;
    INT32              commFd = INVALID_CONNECTION;
    UINT8              fdHandle;
    UINT32             rcvMsgLen;
    CHARPTR            msgPtr;
    UINT8              msgId;
    UINT64             statusNumber = 255;
    UINT64             tableId;
    UINT32             pollTimeInMs;

    THREAD_START_INDEX("TCP_XFER", publicTcpInfo->tcpHandle);
    DPRINT(CAMERA_INITIATION, "tcp transfer: [camera=%d], [handle=%d], [tcpRequest=%d]", publicTcpInfo->tcpInfo.camIndex, publicTcpInfo->tcpHandle,
           publicTcpInfo->tcpInfo.tcpRequest);

    do
    {
        camIndex = publicTcpInfo->tcpInfo.camIndex;
        realCamIndex = GET_STREAM_INDEX(camIndex);
        if (FAIL == AddCmdCamCount(realCamIndex, &fdHandle))
        {
            EPRINT(CAMERA_INITIATION, "fail to add command: [camera=%d]", camIndex);
            tcpResp = TCP_ERROR;
            break;
        }

        /* Get CI polling time in seconds */
        pollTimeInMs = (GetCiPollTimeInSec() * MSEC_IN_ONE_SEC);
        do
        {
            if (SUCCESS == GetCmdFd(realCamIndex, &commFd, &fdHandle, FALSE))
            {
                break;
            }

            /* Fail to get fd. Hence check timeout */
            if (timeOut >= pollTimeInMs)
            {
                /* Fd did not available till timeout */
                GetCmdFd(realCamIndex, &commFd, &fdHandle, TRUE);
                EPRINT(CAMERA_INITIATION, "get fd timeout: [camera=%d], [handle=%d], [tcpRequest=%d]", camIndex, publicTcpInfo->tcpHandle,
                       publicTcpInfo->tcpInfo.tcpRequest);
                break;
            }

            /* Update timeout and sleep */
            timeOut += 20;
            usleep(20000);

        } while (TRUE);

        /* Is error occurred or fd not available? */
        if (INVALID_CONNECTION == commFd)
        {
            tcpResp = TCP_ERROR;
            break;
        }

        tcpRespData[publicTcpInfo->tcpHandle].connFd = commFd;
        snprintf(commMsg, CI_MAX_COMM_DATA, "%s", publicTcpInfo->tcpInfo.commandString);
        DPRINT(CAMERA_INITIATION, "request cmd msg: [camera=%d], [msg=%s], [fd=%d]", camIndex, commMsg, commFd);

        if (FAIL == SendToSocket(commFd, (UINT8PTR)commMsg, strlen(commMsg), GetCiPollDuration()))
        {
            tcpResp = TCP_CLOSE_ON_ERROR;
            EPRINT(CAMERA_INITIATION, "fail to send msg: [camera=%d]", camIndex);
            break;
        }

        if (SUCCESS != RecvMessage(commFd, commMsg, &rcvMsgLen, SOM, EOM, CI_MAX_COMM_DATA - 1, GetCiPollDuration()))
        {
            tcpResp = TCP_CLOSE_ON_ERROR;
            EPRINT(CAMERA_INITIATION, "fail to rcv msg: timeout or connection closed: [camera=%d], [fd=%d]", camIndex, commFd);
            break;
        }

        commMsg[rcvMsgLen] = '\0';
        DPRINT(CAMERA_INITIATION, "resp cmd msg: [camera=%d], [tcpRequest=%d], [msg=%s]", camIndex, publicTcpInfo->tcpInfo.tcpRequest, commMsg);
        msgPtr = commMsg + 1;

        if (findHeaderIndexTcpClient(&msgPtr, &msgId) != CMD_SUCCESS)
        {
            tcpResp = TCP_CLOSE_ON_ERROR;
            EPRINT(CAMERA_INITIATION, "fail to get header index: [camera=%d]", camIndex);
            break;
        }

        switch (msgId)
        {
            case CI_RPL_CMD:
            {
                if (ParseStringGetVal(&msgPtr, &statusNumber, 1, FSP) != SUCCESS)
                {
                    EPRINT(CAMERA_INITIATION, "fail to parse tcp cmd resp: [camera=%d]", camIndex);
                    tcpRespData[publicTcpInfo->tcpHandle].ciCmdResp = MAX_CI_CMD_STATUS;
                    break;
                }

                tcpRespData[publicTcpInfo->tcpHandle].camIndex = camIndex;
                tcpRespData[publicTcpInfo->tcpHandle].ciCmdResp = statusNumber;
                if (CI_CMD_SUCCESS != statusNumber)
                {
                    EPRINT(CAMERA_INITIATION, "fail response rcvd for tcp cmd req: [camera=%d], [tcpRequest=%d], [status=%lld]", camIndex,
                           publicTcpInfo->tcpInfo.tcpRequest, statusNumber);
                    tcpResp = TCP_CLOSE_ON_ERROR;
                    break;
                }

                tcpResp = TCP_CLOSE_ON_SUCCESS;
                DPRINT(CAMERA_INITIATION, "success resp rcvd for tcp cmd: [camera=%d], [tcpRequest=%d]", camIndex, publicTcpInfo->tcpInfo.tcpRequest);
                CI_CMD_PARSER_e parserId = getCmdParserIndex(publicTcpInfo->tcpInfo.tcpRequest);
                if (parserId < CI_CMD_PARSER_MAX)
                {
                    if (FAIL == ciCmdRespParserFuncPtr[parserId](&msgPtr, publicTcpInfo->tcpHandle))
                    {
                        EPRINT(CAMERA_INITIATION, "fail to parse response: [camera=%d], [tcpRequest=%d]", camIndex,
                               publicTcpInfo->tcpInfo.tcpRequest);
                        tcpResp = TCP_CLOSE_ON_ERROR;
                        break;
                    }
                }
                else
                {
                    if (publicTcpInfo->tcpResponseCb != NULL)
                    {
                        tcpRespData[publicTcpInfo->tcpHandle].tcpResp = tcpResp;
                        publicTcpInfo->tcpResponseCb(publicTcpInfo->tcpHandle, &tcpRespData[publicTcpInfo->tcpHandle]);
                    }
                }
            }
            break;

            case CI_RPL_CFG:
            {
                if (ParseStringGetVal(&msgPtr, &statusNumber, 1, FSP) != SUCCESS)
                {
                    EPRINT(CAMERA_INITIATION, "fail to parse tcp cfg resp: [camera=%d]", camIndex);
                    tcpRespData[publicTcpInfo->tcpHandle].ciCmdResp = MAX_CI_CMD_STATUS;
                    break;
                }

                tcpRespData[publicTcpInfo->tcpHandle].ciCmdResp = statusNumber;
                tcpRespData[publicTcpInfo->tcpHandle].camIndex = camIndex;
                if (CI_CMD_SUCCESS != statusNumber)
                {
                    EPRINT(CAMERA_INITIATION, "fail resp rcvd for tcp cfg req: [camera=%d], [tcpRequest=%d], [status=%lld]", camIndex,
                           publicTcpInfo->tcpInfo.tcpRequest, statusNumber);
                    tcpResp = TCP_CLOSE_ON_ERROR;
                    break;
                }

                DPRINT(CAMERA_INITIATION, "success resp rcvd for tcp cfg: [camera=%d], [tcpRequest=%d]", camIndex, publicTcpInfo->tcpInfo.tcpRequest);
                tcpResp = TCP_CLOSE_ON_SUCCESS;
                if (CI_GET_CFG != publicTcpInfo->tcpInfo.tcpRequest)
                {
                    tcpRespData[publicTcpInfo->tcpHandle].tcpResp = tcpResp;
                    publicTcpInfo->tcpResponseCb(publicTcpInfo->tcpHandle, &tcpRespData[publicTcpInfo->tcpHandle]);
                    break;
                }

                /* For get config request */
                if (SOT != *msgPtr++)
                {
                    tcpResp = TCP_CLOSE_ON_ERROR;
                    EPRINT(CAMERA_INITIATION, "start of table not found in tcp cfg: [camera=%d]", camIndex);
                    break;
                }

                /* Parse table id */
                if (ParseStringGetVal(&msgPtr, &tableId, 1, FSP) == FAIL)
                {
                    tcpResp = TCP_CLOSE_ON_ERROR;
                    EPRINT(CAMERA_INITIATION, "fail to get tcp cfg table: [camera=%d]", camIndex);
                    break;
                }

                /* Get camera config table to our config parser index */
                CI_CNFG_PARSER_e parserId = getCnfgParserIndex(tableId);
                if (parserId >= MAX_CI_CFG_TABLE_PARSER)
                {
                    tcpResp = TCP_CLOSE_ON_ERROR;
                    EPRINT(CAMERA_INITIATION, "tcp cfg parser not found: [camera=%d], [tableId=%lld]", camIndex, tableId);
                    break;
                }

                /* Parse the config */
                if (FAIL == ciCnfgRespParserFuncPtr[parserId](&msgPtr, publicTcpInfo->tcpHandle))
                {
                    tcpResp = TCP_CLOSE_ON_ERROR;
                    EPRINT(CAMERA_INITIATION, "fail to parse response: [camera=%d], [tcpRequest=%d]", camIndex, publicTcpInfo->tcpInfo.tcpRequest);
                    break;
                }
            }
            break;

            default:
            {
                EPRINT(CAMERA_INITIATION, "invld msgId rcvd: [camera=%d], [msgId%d]", camIndex, msgId);
            }
            break;
        }

    } while (0);

    if ((NULL != publicTcpInfo->tcpResponseCb) && (tcpResp != TCP_CLOSE_ON_SUCCESS))
    {
        tcpRespData[publicTcpInfo->tcpHandle].camIndex = camIndex;
        tcpRespData[publicTcpInfo->tcpHandle].tcpResp = tcpResp;
        publicTcpInfo->tcpResponseCb(publicTcpInfo->tcpHandle, &tcpRespData[publicTcpInfo->tcpHandle]);
    }

    if (INVALID_CONNECTION != commFd)
    {
        /* Do not close the fd for start aud out req, as we required to send audio on that FD */
        if ((CI_SRT_AUD_OUT == publicTcpInfo->tcpInfo.tcpRequest) && (tcpResp == TCP_CLOSE_ON_SUCCESS))
        {
            CloseCamCmdFd(realCamIndex, fdHandle, TRUE);
        }
        else
        {
            CloseCamCmdFd(realCamIndex, fdHandle, FALSE);
        }
    }

    MUTEX_LOCK(tcpReqListMutex);
    publicTcpInfo->requestStatus = FREE;
    MUTEX_UNLOCK(tcpReqListMutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a thread is used for image transfer.
 * @param   tcpSessionPtr
 * @return
 */
static VOIDPTR tcpTransferImage(VOIDPTR tcpSessionPtr)
{
    TCP_CLIENT_INFO_t *publicTcpInfo = (TCP_CLIENT_INFO_t *)tcpSessionPtr;
    TCP_RESPONSE_e     tcpResp = TCP_CLOSE_ON_SUCCESS;
    CHAR               commMsg[MAX_LEN_IMG_RCV];
    UINT8              camIndex = MAX_CAMERA, realCamIndex = MAX_CAMERA;
    UINT8              timeOut = 0;
    INT32              commFd = INVALID_CONNECTION;
    UINT8              fdHandle;
    UINT32             rcvMsgLen = 0;
    CHARPTR            msgPtr;
    UINT8              msgId;
    UINT64             statusNumber = 255;
    BOOL               isFirstFrame = FALSE;
    BOOL               imgRevError = TRUE;
    CHAR               imgData[MAX_SNAP_SHOT_SIZE];
    CHARPTR            imgDataPtr = imgData;
    UINT32             imgLen = MAX_SNAP_SHOT_SIZE;
    UINT16             diffLen = 0;
    INT64              totalImgLenRcv = 0;
    UINT32             revImageLen = 0;
    CHAR               imgLength[MAX_IMG_LEN];
    CHAR               strHexLen[MAX_IMG_LEN];
    UINT8              pollTimeInSec;

    THREAD_START_INDEX("TCP_IMG_XR", publicTcpInfo->tcpHandle);
    DPRINT(CAMERA_INITIATION, "tcp image transfer started: [camera=%d]", publicTcpInfo->tcpInfo.camIndex);

    do
    {
        camIndex = publicTcpInfo->tcpInfo.camIndex;
        realCamIndex = GET_STREAM_INDEX(camIndex);
        if (FAIL == AddCmdCamCount(realCamIndex, &fdHandle))
        {
            tcpResp = TCP_CLOSE_ON_ERROR;
            break;
        }

        /* Get CI polling time in seconds */
        pollTimeInSec = GetCiPollTimeInSec();
        do
        {
            if (SUCCESS == GetCmdFd(realCamIndex, &commFd, &fdHandle, FALSE))
            {
                break;
            }

            /* Fail to get fd. Hence check timeout */
            if (pollTimeInSec == timeOut)
            {
                /* Fd did not available till timeout */
                GetCmdFd(realCamIndex, &commFd, &fdHandle, TRUE);
                EPRINT(CAMERA_INITIATION, "image get fd timeout: [camera=%d], [handle=%d]", camIndex, fdHandle);
                break;
            }

            /* Update timeout and sleep */
            timeOut++;
            sleep(1);

        } while (TRUE);

        /* Is error occurred or fd not available? */
        if (INVALID_CONNECTION == commFd)
        {
            tcpResp = TCP_CLOSE_ON_ERROR;
            break;
        }

        snprintf(commMsg, MAX_LEN_IMG_RCV, "%s", publicTcpInfo->tcpInfo.commandString);
        DPRINT(CAMERA_INITIATION, "img xfer msg: [camera=%d], [msg=%s], [fd=%d]", camIndex, commMsg, commFd);
        if (FAIL == SendToSocket(commFd, (UINT8PTR)commMsg, strlen(commMsg), GetCiPollDuration()))
        {
            tcpResp = TCP_CLOSE_ON_ERROR;
            EPRINT(CAMERA_INITIATION, "fail to send img xfer msg: [camera=%d]", camIndex);
            break;
        }

        memset(commMsg, '\0', sizeof(commMsg));
        while (TRUE)
        {
            rcvMsgLen = 0;
            if (imgRevError == FALSE)
            {
                break;
            }

            if (isFirstFrame == FALSE)
            {
                revImageLen = 512;
            }
            else
            {
                DPRINT(CAMERA_INITIATION, "image recv progress: [camera=%d], [totalImgLenRcv=%lld], [imgLen=%d]", camIndex, totalImgLenRcv, imgLen);
                if (totalImgLenRcv > MAX_LEN_IMG_RCV)
                {
                    revImageLen = MAX_LEN_IMG_RCV;
                }
                else
                {
                    revImageLen = totalImgLenRcv;
                }
            }

            if (SUCCESS != RecvFrame(commFd, commMsg, &rcvMsgLen, revImageLen, GetCiPollDuration(), 0))
            {
                EPRINT(CAMERA_INITIATION, "fail to recv image msg frame: [camera=%d]", camIndex);
                tcpResp = TCP_CLOSE_ON_ERROR;
                break;
            }

            if (FALSE == isFirstFrame)
            {
                isFirstFrame = TRUE;
                msgPtr = commMsg + 1;
                if (findHeaderIndexTcpClient(&msgPtr, &msgId) != CMD_SUCCESS)
                {
                    tcpResp = TCP_CLOSE_ON_ERROR;
                    EPRINT(CAMERA_INITIATION, "fail to get header index: [camera=%d]", camIndex);
                    break;
                }

                switch (msgId)
                {
                    case CI_RPL_CMD:
                    {
                        if (SUCCESS != ParseStringGetVal(&msgPtr, &statusNumber, 1, FSP))
                        {
                            EPRINT(CAMERA_INITIATION, "fail to parse tcp img resp: [camera=%d]", camIndex);
                            break;
                        }

                        if (CI_CMD_SUCCESS != statusNumber)
                        {
                            tcpResp = TCP_CLOSE_ON_ERROR;
                            EPRINT(CAMERA_INITIATION, "fail resp rcvd for tcp img req: [camera=%d], [status=%lld]", camIndex, statusNumber);
                            imgRevError = FALSE;
                            break;
                        }

                        if (ParseStr(&msgPtr, FSP, imgLength, MAX_IMG_LEN) != SUCCESS)
                        {
                            tcpResp = TCP_CLOSE_ON_ERROR;
                            imgRevError = FALSE;
                            EPRINT(CAMERA_INITIATION, "fail to parse image length: [camera=%d]", camIndex);
                            break;
                        }

                        snprintf(strHexLen, MAX_IMG_LEN, "0x%.6s", imgLength); /* Note: Length decided in SAD (6bytes) */
                        statusNumber = strtoll(strHexLen, NULL, 16);
                        totalImgLenRcv = (UINT64)statusNumber;
                        DPRINT(CAMERA_INITIATION, "image length: [camera=%d], [length=%lld]", camIndex, totalImgLenRcv);

                        if ((totalImgLenRcv >= MAX_SNAP_SHOT_SIZE) || (rcvMsgLen == 0))
                        {
                            tcpResp = TCP_CLOSE_ON_ERROR;
                            imgRevError = FALSE;
                            EPRINT(CAMERA_INITIATION, "total len more than buffer: [camera=%d], [length=%d]", camIndex, rcvMsgLen);
                            break;
                        }

                        diffLen = msgPtr - commMsg;
                        if ((INT32)(rcvMsgLen - diffLen) >= totalImgLenRcv)
                        {
                            // success
                            imgRevError = FALSE;
                        }

                        if (FALSE == imgRevError)
                        {
                            if (((INT64)imgLen - (rcvMsgLen - diffLen - 2)) >= 0)
                            {
                                totalImgLenRcv -= (rcvMsgLen - diffLen - 2);
                                memcpy(imgDataPtr, msgPtr, (rcvMsgLen - diffLen - 2));
                                imgLen -= (rcvMsgLen - diffLen - 2);
                            }
                            else
                            {
                                EPRINT(CAMERA_INITIATION, "max image length reached: [camera=%d]", camIndex);
                                imgRevError = FALSE;
                                break;
                            }
                        }
                        else
                        {
                            if (((INT64)imgLen - (rcvMsgLen - diffLen)) >= 0)
                            {
                                totalImgLenRcv -= (rcvMsgLen - diffLen);
                                memcpy(imgDataPtr, msgPtr, (rcvMsgLen - diffLen));
                                imgLen -= (rcvMsgLen - diffLen);
                            }
                            else
                            {
                                EPRINT(CAMERA_INITIATION, "max image length reached: [camera=%d]", camIndex);
                                imgRevError = FALSE;
                                break;
                            }
                        }

                        imgDataPtr += (rcvMsgLen - diffLen);
                    }
                    break;

                    default:
                    {
                        EPRINT(CAMERA_INITIATION, "invld msgId rcvd: [camera=%d], [msgId=%d]", camIndex, msgId);
                    }
                    break;
                }
            }
            else
            {
                if (rcvMsgLen != 0)
                {
                    if ((INT32)(imgLen - rcvMsgLen) >= 0)
                    {
                        totalImgLenRcv -= rcvMsgLen;
                        memcpy(imgDataPtr, commMsg, rcvMsgLen);
                        imgLen -= (rcvMsgLen);
                        imgDataPtr += (rcvMsgLen);
                    }
                    else
                    {
                        DPRINT(CAMERA_INITIATION, "max image length reached: [camera=%d]", camIndex);
                        imgRevError = FALSE;
                        break;
                    }
                }
                else
                {
                    EPRINT(CAMERA_INITIATION, "error in recv length: [camera=%d]", camIndex);
                }
            }

            if (TCP_CLOSE_ON_ERROR == tcpResp)
            {
                break;
            }

            if (totalImgLenRcv <= 0)
            {
                DPRINT(CAMERA_INITIATION, "whole img file rcvd: [camera=%d]", camIndex);
                break;
            }
        }

        commMsg[rcvMsgLen] = '\0';

    } while (0);

    if (NULL != publicTcpInfo->tcpResponseCb)
    {
        tcpRespData[publicTcpInfo->tcpHandle].tcpResp = tcpResp;
        tcpRespData[publicTcpInfo->tcpHandle].camIndex = camIndex;
        tcpRespData[publicTcpInfo->tcpHandle].storagePtr = imgData;
        tcpRespData[publicTcpInfo->tcpHandle].storageLen = (MAX_SNAP_SHOT_SIZE - imgLen);
        DPRINT(CAMERA_INITIATION, "storage image length: [camera=%d], [length=%d]", camIndex, tcpRespData[publicTcpInfo->tcpHandle].storageLen);
        publicTcpInfo->tcpResponseCb(publicTcpInfo->tcpHandle, &tcpRespData[publicTcpInfo->tcpHandle]);
    }

    if (INVALID_CONNECTION != commFd)
    {
        CloseCamCmdFd(realCamIndex, fdHandle, FALSE);
    }

    MUTEX_LOCK(tcpReqListMutex);
    publicTcpInfo->requestStatus = FREE;
    MUTEX_UNLOCK(tcpReqListMutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is used for parsing the table index.
 * @param   tableId
 * @return  parser index
 */
static CI_CNFG_PARSER_e getCnfgParserIndex(UINT64 tableId)
{
    switch (tableId)
    {
        case CI_STREAM_PROFILE_CFG:
            return CI_STREAM_PROFILE_CFG_PARSER;

        case CI_OVERLAY_CFG:
            return CI_OVERLAY_CFG_PARSER;

        case CI_MOTION_DETECT_CFG:
            return CI_MOTION_DETECT_CFG_PARSER;

        case CI_APPEARANCE_CFG:
            return CI_APPEARANCE_CFG_PARSER;

        case CI_ADV_APPEARANCE_CFG:
            return CI_ADV_APPEARANCE_CFG_PARSER;

        case CI_DAY_NIGHT_CFG:
            return CI_DAY_NIGHT_CFG_PARSER;

        default:
            return MAX_CI_CFG_TABLE_PARSER;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a function is used for stream config parser.
 * @param   respStrPtr
 * @param   tcpHandle
 * @return  SUCCESS/FAIL
 */
static BOOL streamProfileCnfgParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle)
{

    UINT64                     fieldId;
    UINT64                     indexId;
    BOOL                       retVal = SUCCESS;
    BOOL                       writeStatus = SUCCESS;
    UINT64                     tempField = 0;
    CHAR                       tmpCodec[7] = {'\0'};
    CHAR                       tmpRes[10] = {'\0'};
    MATRIX_IP_CAM_RESOLUTION_e resolutionNo;

    CI_STREAM_PROFILE_CONFIG_t *pStreamProfileConfig = malloc(sizeof(CI_STREAM_PROFILE_CONFIG_t));
    if (NULL == pStreamProfileConfig)
    {
        return FAIL;
    }

    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);
    tcpRespData[tcpHandle].ciCfgData = pStreamProfileConfig;

    while (**respStrPtr != EOT)
    {
        if (**respStrPtr != SOI)
        {
            continue;
        }

        // increment for SOI
        (*respStrPtr)++;

        // Get Index Id
        if (ParseStringGetVal(respStrPtr, &indexId, 1, FSP) == FAIL)
        {
            retVal = FAIL;
            break;
        }

        if (indexId > 4)  //  Index should be 4 as camera support 4 profiles, so in stream profile config 4 index will be there.
        {
            retVal = FAIL;
            EPRINT(CAMERA_INITIATION, "max index id reached");
            break;
        }

        while (**respStrPtr != EOI)
        {
            // Get field string
            if (ParseStringGetVal(respStrPtr, &fieldId, 1, FVS) == FAIL)
            {
                retVal = FAIL;
                writeStatus = FAIL;
                break;
            }

            switch ((CI_STREAM_CFG_FIELD_e)fieldId)
            {
                case CI_STREAM_CFG_NAME:
                    writeStatus = ParseStr(respStrPtr, FSP, pStreamProfileConfig->name, MAX_PROFILE_NAME_LENGTH);
                    break;

                case CI_STREAM_CFG_CODEC:
                    writeStatus = ParseStr(respStrPtr, FSP, tmpCodec, MAX_ENCODER_NAME_LEN);
                    if (strstr(tmpCodec, codecStrForSetMatrixIP[0]) != NULL)
                    {
                        snprintf(pStreamProfileConfig->codec, MAX_ENCODER_NAME_LEN, "%s", actualCodecStr[VIDEO_H264]);
                    }
                    else if (strstr(tmpCodec, codecStrForSetMatrixIP[1]) != NULL)
                    {
                        snprintf(pStreamProfileConfig->codec, MAX_ENCODER_NAME_LEN, "%s", actualCodecStr[VIDEO_MJPG]);
                    }
                    else if (strstr(tmpCodec, codecStrForSetMatrixIP[2]) != NULL)
                    {
                        snprintf(pStreamProfileConfig->codec, MAX_ENCODER_NAME_LEN, "%s", actualCodecStr[VIDEO_H265]);
                    }
                    break;

                case CI_STREAM_CFG_RESL:
                    writeStatus = ParseStr(respStrPtr, FSP, tmpRes, MAX_RESOLUTION_NAME_LEN);
                    if (GetResolutionNoGetProfileforMatrixIpCamera(tmpRes, &resolutionNo) == SUCCESS)
                    {
                        snprintf(pStreamProfileConfig->resolution, MAX_RESOLUTION_NAME_LEN, "%s", resolutionStrForMatrixIP[resolutionNo]);
                    }
                    break;

                case CI_STREAM_CFG_FPS:
                    writeStatus = ParseStringGetVal(respStrPtr, &tempField, 1, FSP);
                    pStreamProfileConfig->fps = (INT16)tempField;
                    break;

                case CI_STREAM_CFG_BITCTRL:
                    writeStatus = ParseStr(respStrPtr, FSP, pStreamProfileConfig->bitRateCtl, MAX_BITRATE_CTRL_NAME_LENGTH);
                    break;

                case CI_STREAM_CFG_BITRATE:
                    writeStatus = ParseStringGetVal(respStrPtr, &tempField, 1, FSP);
                    pStreamProfileConfig->bitRate = (INT32)tempField;
                    break;

                case CI_STREAM_CFG_IMGQLTY:
                    writeStatus = ParseStringGetVal(respStrPtr, &tempField, 1, FSP);
                    pStreamProfileConfig->imageQuality = (UINT8)tempField;
                    break;

                case CI_STREAM_CFG_GOP:
                    writeStatus = ParseStringGetVal(respStrPtr, &tempField, 1, FSP);
                    pStreamProfileConfig->gop = (INT16)tempField;
                    break;

                case CI_STREAM_CFG_ADPTSTRM:
                    writeStatus = ParseStringGetVal(respStrPtr, &tempField, 1, FSP);
                    pStreamProfileConfig->adaptiveStream = (UINT8)tempField;
                    break;

                case CI_STREAM_CFG_BWOPT:
                    writeStatus = ParseStringGetVal(respStrPtr, &tempField, 1, FSP);
                    pStreamProfileConfig->bandWidthOpt = (UINT8)tempField;
                    break;

                case CI_STREAM_CFG_AUD:
                    writeStatus = ParseStringGetVal(respStrPtr, &tempField, 1, FSP);
                    pStreamProfileConfig->audio = (UINT8)tempField;
                    break;

                default:
                    break;
            }

            if (writeStatus == FAIL)
            {
                EPRINT(CAMERA_INITIATION, "fail to parse: [tcpHandle=%d], [fieldId=%llu]", tcpHandle, fieldId);
                break;
            }
        }

        // ignore EOI
        (*respStrPtr)++;

        if (writeStatus == SUCCESS)
        {
            tcpRespData[tcpHandle].tcpResp = TCP_CLOSE_ON_SUCCESS;
            tcpClientInfo[tcpHandle].tcpResponseCb(tcpHandle, &tcpRespData[tcpHandle]);
        }
    }

    // ignore EOT
    (*respStrPtr)++;
    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a function is used for overlay Config Parser.
 * @param   respStrPtr
 * @param   tcpHandle
 * @return  SUCCESS / FAIL
 */
static BOOL overlayCnfgParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle)
{
    UINT64 fieldId = 0;
    UINT64 indexId = 0;
    UINT64 fieldVal = 0, tempVal = 0;
    BOOL   retVal = SUCCESS;
    BOOL   writeStatus = SUCCESS;

    CI_OVERLAY_CONFIG_t *pOverLayConfig = malloc(sizeof(CI_OVERLAY_CONFIG_t));
    if (NULL == pOverLayConfig)
    {
        return FAIL;
    }

    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);
    tcpRespData[tcpHandle].ciCfgData = pOverLayConfig;

    while (**respStrPtr != EOT)
    {
        if (**respStrPtr != SOI)
        {
            continue;
        }

        // increment for SOI
        (*respStrPtr)++;

        // Get Index Id
        if (ParseStringGetVal(respStrPtr, &indexId, 1, FSP) == FAIL)
        {
            retVal = FAIL;
            break;
        }

        if (indexId > 1)
        {
            retVal = FAIL;
            EPRINT(CAMERA_INITIATION, "max index id reached");
            break;
        }

        while (**respStrPtr != EOI)
        {
            // Get field string
            if (ParseStringGetVal(respStrPtr, &fieldId, 1, FVS) == FAIL)
            {
                retVal = FAIL;
                writeStatus = FAIL;
                break;
            }

            switch (fieldId)
            {
                case CI_OVERLAY_CFG_ENABLE:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->enableDateTime = (UINT8)fieldVal;
                    break;

                case CI_OVERLAY_CFG_DATE:
                    writeStatus = ParseStr(respStrPtr, FSP, pOverLayConfig->dateFormate, MAX_DATE_FORMAT_NAME_LENGTH);
                    break;

                case CI_OVERLAY_CFG_TIME:
                    writeStatus = ParseStr(respStrPtr, FSP, pOverLayConfig->timeFormate, MAX_TIMEFORMAT_NAME_LENGTH);
                    break;

                case CI_OVERLAY_CFG_DISP:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->display = (OSD_POS_e)fieldVal;
                    break;

                case CI_OVERLAY_CFG_TXT_ENABLE:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->enableText = (UINT8)fieldVal;
                    break;

                case CI_OVERLAY_CFG_TXT0:
                    writeStatus = ParseStr(respStrPtr, FSP, pOverLayConfig->text[0], MAX_OVERLAY_TEXT_LENGTH);
                    break;

                case CI_OVERLAY_CFG_TXT_POS0:
                    writeStatus = ParseStringGetVal(respStrPtr, &tempVal, 1, FSP);
                    break;

                case CI_OVERLAY_CFG_ENBIMG:
                    writeStatus = ParseStringGetVal(respStrPtr, &tempVal, 1, FSP);
                    break;

                case CI_OVERLAY_CFG_DISPIMG:
                    writeStatus = ParseStringGetVal(respStrPtr, &tempVal, 1, FSP);
                    break;

                case CI_OVERLAY_CFG_ENBPRIMASK:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->enablePrivacyMask = (UINT8)fieldVal;
                    break;

                case CI_OVERLAY_CFG_ENBMASK1:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[0].maskEnable = (UINT8)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTXMASK1:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[0].startXPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTYMASK1:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[0].startYPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_WDTHMASK1:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[0].width = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_HTMASK1:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[0].height = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_ENBMASK2:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[1].maskEnable = (UINT8)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTXMASK2:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[1].startXPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTYMASK2:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[1].startYPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_WDTHMASK2:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[1].width = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_HTMASK2:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[1].height = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_ENBMASK3:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[2].maskEnable = (UINT8)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTXMASK3:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[2].startXPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTYMASK3:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[2].startYPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_WDTHMASK3:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[2].width = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_HTMASK3:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[2].height = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_ENBMASK4:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[3].maskEnable = (UINT8)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTXMASK4:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[3].startXPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTYMASK4:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[3].startYPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_WDTHMASK4:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[3].width = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_HTMASK4:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[3].height = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_ENBMASK5:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[4].maskEnable = (UINT8)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTXMASK5:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[4].startXPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTYMASK5:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[4].startYPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_WDTHMASK5:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[4].width = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_HTMASK5:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[4].height = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_ENBMASK6:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[5].maskEnable = (UINT8)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTXMASK6:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[5].startXPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTYMASK6:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[5].startYPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_WDTHMASK6:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[5].width = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_HTMASK6:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[5].height = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_ENBMASK7:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[6].maskEnable = (UINT8)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTXMASK7:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[6].startXPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTYMASK7:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[6].startYPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_WDTHMASK7:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[6].width = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_HTMASK7:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[6].height = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_ENBMASK8:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[7].maskEnable = (UINT8)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTXMASK8:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[7].startXPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_STRTYMASK8:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[7].startYPoint = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_WDTHMASK8:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[7].width = (UINT32)fieldVal;
                    break;

                case CI_OVERLAY_CFG_HTMASK8:
                    writeStatus = ParseStringGetVal(respStrPtr, &fieldVal, 1, FSP);
                    pOverLayConfig->privacyMaskArea[7].height = (UINT32)fieldVal;
                    break;

                default:
                    break;
            }
        }

        // ignore EOI
        (*respStrPtr)++;

        if (writeStatus == SUCCESS)
        {
            tcpRespData[tcpHandle].tcpResp = TCP_CLOSE_ON_SUCCESS;
            tcpClientInfo[tcpHandle].tcpResponseCb(tcpHandle, &tcpRespData[tcpHandle]);
        }
    }

    (*respStrPtr)++;
    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a used for motion config parser.
 * @param   respStrPtr
 * @param   tcpHandle
 * @return  SUCCESS / FAIL
 */
static BOOL motionCnfgParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle)
{
    UINT64  fieldId;
    UINT64  indexId;
    UINT64  tempField = 0;
    BOOL    retVal = SUCCESS;
    BOOL    writeStatus = SUCCESS;
    CHAR  **convertedMatrixBuf;
    UINT16  packedLen = 0;
    CHAR    packedToPass[MOTION_AREA_BLOCK_BYTES * 2];
    UINT32  packByte;
    UINT8   cnt = 0;
    CHAR    tempData[(MOTION_AREA_BLOCK_BYTES * 2) + 1] = {'\0'};
    CHARPTR dataPtr = NULL;

    MOTION_BLOCK_METHOD_PARAM_t *pMotionConfig = malloc(sizeof(MOTION_BLOCK_METHOD_PARAM_t));
    if (NULL == pMotionConfig)
    {
        return FAIL;
    }

    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);
    tcpRespData[tcpHandle].ciCfgData = pMotionConfig;
    memset(pMotionConfig, 0, sizeof(MOTION_BLOCK_METHOD_PARAM_t));
    pMotionConfig->sensitivity = 5;
    pMotionConfig->noMotionSupportF = FALSE;
    pMotionConfig->isNoMotionEvent = FALSE;
    pMotionConfig->noMotionDuration = 5;

    while (**respStrPtr != EOT)
    {
        if (**respStrPtr != SOI)
        {
            continue;
        }

        // increment for SOI
        (*respStrPtr)++;

        // Get Index Id
        if (ParseStringGetVal(respStrPtr, &indexId, 1, FSP) == FAIL)
        {
            EPRINT(CAMERA_INITIATION, "fail to parse index id");
            retVal = FAIL;
            break;
        }

        if (indexId > 1)
        {
            retVal = FAIL;
            EPRINT(CAMERA_INITIATION, "max index id reached");
            break;
        }

        while (**respStrPtr != EOI)
        {
            // Get field string
            if (ParseStringGetVal(respStrPtr, &fieldId, 1, FVS) == FAIL)
            {
                retVal = FAIL;
                writeStatus = FAIL;
                EPRINT(CAMERA_INITIATION, "fail to parse index id");
                break;
            }

            switch (fieldId)
            {
                case CI_MOTION_CFG_CELL:
                    writeStatus = ParseStr(respStrPtr, FSP, tempData, sizeof(tempData));
                    dataPtr = tempData;
                    convertedMatrixBuf = (CHAR **)Allocate2DArray(36, 44, sizeof(CHAR));
                    if (NULL == convertedMatrixBuf)
                    {
                        retVal = FAIL;
                        EPRINT(CAMERA_INITIATION, "fail to alloc memory");
                        break;
                    }

                    for (cnt = 0; cnt < MOTION_AREA_BLOCK_BYTES; cnt++)
                    {
                        sscanf(dataPtr, "%02x", &packByte);
                        packedToPass[cnt] = (UINT8)packByte;
                        dataPtr = (dataPtr + 2);
                    }

                    UnPackGridGeneral(packedToPass, 44, 36, convertedMatrixBuf, MOTION_AREA_BLOCK_BYTES);
                    PackGridGeneral(convertedMatrixBuf, 44, 36, pMotionConfig->blockBitString, &packedLen);
                    Free2DArray((void **)convertedMatrixBuf, 36);
                    break;

                case CI_MOTION_CFG_SENS:
                    writeStatus = ParseStringGetVal(respStrPtr, &tempField, 1, FSP);
                    pMotionConfig->sensitivity = (UINT32)tempField / 10;
                    break;

                case CI_MOTION_CFG_IS_NO_MOTION_EVNT:
                    writeStatus = ParseStringGetVal(respStrPtr, &tempField, 1, FSP);
                    pMotionConfig->isNoMotionEvent = GET_BOOL_VALUE((UINT32)tempField);
                    pMotionConfig->noMotionSupportF = TRUE;
                    break;

                case CI_MOTION_CFG_NO_MOTION_DURATION:
                    writeStatus = ParseStringGetVal(respStrPtr, &tempField, 1, FSP);
                    pMotionConfig->noMotionDuration = (UINT32)tempField;
                    break;

                default:
                    /* Skip other param from parsing */
                    writeStatus = ParseStr(respStrPtr, FSP, tempData, sizeof(tempData));
                    break;
            }
        }

        // ignore EOI
        (*respStrPtr)++;

        if (writeStatus == SUCCESS)
        {
            tcpRespData[tcpHandle].tcpResp = TCP_CLOSE_ON_SUCCESS;
            tcpClientInfo[tcpHandle].tcpResponseCb(tcpHandle, &tcpRespData[tcpHandle]);
        }
    }

    (*respStrPtr)++;
    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Image setting appearance config parser
 * @param   respStrPtr
 * @param   tcpHandle
 * @return
 */
static BOOL appearanceCnfgParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle)
{
    UINT64 parseValue;
    BOOL   writeStatus = FAIL;

    /* Allocate memory for image setting param */
    IMAGE_CAPABILITY_INFO_t *pImageCapsInfo = malloc(sizeof(IMAGE_CAPABILITY_INFO_t));
    if (NULL == pImageCapsInfo)
    {
        /* Fail to alloc memory */
        return FAIL;
    }

    /* Free memory if already allocated */
    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);
    tcpRespData[tcpHandle].ciCfgData = pImageCapsInfo;

    /* Reset capability info before using it */
    memset(pImageCapsInfo, 0, sizeof(IMAGE_CAPABILITY_INFO_t));
    while (**respStrPtr != EOT)
    {
        /* Search for start of index */
        if (**respStrPtr != SOI)
        {
            continue;
        }

        /* Increment for SOI */
        (*respStrPtr)++;

        /* Get the index */
        if (ParseStringGetVal(respStrPtr, &parseValue, 1, FSP) == FAIL)
        {
            break;
        }

        /* Index should start from 0 or 1 */
        if (parseValue > 1)
        {
            EPRINT(CAMERA_INITIATION, "index max reached: [indexId=%d]", (UINT32)parseValue);
            break;
        }

        /* Search for end of index */
        while (**respStrPtr != EOI)
        {
            /* Get field string */
            if (ParseStringGetVal(respStrPtr, &parseValue, 1, FVS) == FAIL)
            {
                break;
            }

            switch (parseValue)
            {
                case CI_APPEARANCE_CNFG_BRIGHTNESS:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->brightness.value = parseValue;
                }
                break;

                case CI_APPEARANCE_CNFG_CONSTRAST:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->contrast.value = parseValue;
                }
                break;

                case CI_APPEARANCE_CNFG_SATURATION:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->saturation.value = parseValue;
                }
                break;

                case CI_APPEARANCE_CNFG_HUE:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->hue.value = parseValue;
                }
                break;

                case CI_APPEARANCE_CNFG_SHARPNESS:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->sharpness.value = parseValue;
                }
                break;

                case CI_APPEARANCE_CNFG_WHITE_BALANCE:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->whiteBalance.mode = parseValue;
                }
                break;

                default:
                {
                    /* Skip other param from parsing */
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                }
                break;
            }

            /* Break process on failure */
            if (FAIL == writeStatus)
            {
                break;
            }
        }

        /* Break process on failure */
        if (FAIL == writeStatus)
        {
            break;
        }

        /* Ignore EOI */
        (*respStrPtr)++;

        /* Provide parsed data in callback for further processing */
        tcpRespData[tcpHandle].tcpResp = TCP_CLOSE_ON_SUCCESS;
        tcpClientInfo[tcpHandle].tcpResponseCb(tcpHandle, &tcpRespData[tcpHandle]);
    }

    /* Ignore EOT */
    (*respStrPtr)++;

    /* Free memory if allocated */
    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);

    /* Return parse status */
    return writeStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Image setting advance appearance config parser
 * @param   respStrPtr
 * @param   tcpHandle
 * @return
 */
static BOOL advanceAppearanceCnfgParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle)
{
    UINT64 parseValue;
    BOOL   writeStatus = FAIL;

    /* Allocate memory for image setting param */
    IMAGE_CAPABILITY_INFO_t *pImageCapsInfo = malloc(sizeof(IMAGE_CAPABILITY_INFO_t));
    if (NULL == pImageCapsInfo)
    {
        /* Fail to alloc memory */
        return FAIL;
    }

    /* Free memory if already allocated */
    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);
    tcpRespData[tcpHandle].ciCfgData = pImageCapsInfo;

    /* Reset capability info before using it */
    memset(pImageCapsInfo, 0, sizeof(IMAGE_CAPABILITY_INFO_t));
    while (**respStrPtr != EOT)
    {
        /* Search for start of index */
        if (**respStrPtr != SOI)
        {
            continue;
        }

        /* Increment for SOI */
        (*respStrPtr)++;

        /* Get the index */
        if (ParseStringGetVal(respStrPtr, &parseValue, 1, FSP) == FAIL)
        {
            break;
        }

        /* Index should start from 0 or 1 */
        if (parseValue > 1)
        {
            EPRINT(CAMERA_INITIATION, "index max reached: [indexId=%d]", (UINT32)parseValue);
            break;
        }

        /* Search for end of index */
        while (**respStrPtr != EOI)
        {
            /* Get field string */
            if (ParseStringGetVal(respStrPtr, &parseValue, 1, FVS) == FAIL)
            {
                break;
            }

            switch (parseValue)
            {
                case CI_ADV_APPEARANCE_CNFG_WDR:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->wdr.mode = parseValue;
                }
                break;

                case CI_ADV_APPEARANCE_CNFG_BACKLIGHT:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->backlightControl.mode = parseValue;
                }
                break;

                case CI_ADV_APPEARANCE_CNFG_DWDR_STRENGTH:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->wdrStrength.value = parseValue;
                }
                break;

                case CI_ADV_APPEARANCE_CNFG_EXPOSURE_RATIO_MODE:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->exposureRatioMode.mode = parseValue;
                }
                break;

                case CI_ADV_APPEARANCE_CNFG_EXPOSURE_RATIO:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->exposureRatio.value = parseValue;
                }
                break;

                case CI_ADV_APPEARANCE_CNFG_EXPOSURE_FLICKER:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->flicker.mode = parseValue;
                }
                break;

                case CI_ADV_APPEARANCE_CNFG_EXPOSURE_MODE:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->exposureMode.mode = parseValue;
                }
                break;

                case CI_ADV_APPEARANCE_CNFG_EXPOSURE_GAIN:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->exposureGain.value = parseValue;
                }
                break;

                case CI_ADV_APPEARANCE_CNFG_EXPOSURE_IRIS:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->exposureIris.value = parseValue;
                }
                break;

                case CI_ADV_APPEARANCE_CNFG_NORMAL_LIGHT_GAIN:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->normalLightGain.mode = parseValue;
                }
                break;

                case CI_ADV_APPEARANCE_CNFG_NORMAL_LIGHT_LUMINANCE:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->normalLightLuminance.value = parseValue;
                }
                break;

                default:
                {
                    /* Skip other param from parsing */
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                }
                break;
            }

            /* Break process on failure */
            if (FAIL == writeStatus)
            {
                break;
            }
        }

        /* Break process on failure */
        if (FAIL == writeStatus)
        {
            break;
        }

        /* Ignore EOI */
        (*respStrPtr)++;

        /* Provide parsed data in callback for further processing */
        tcpRespData[tcpHandle].tcpResp = TCP_CLOSE_ON_SUCCESS;
        tcpClientInfo[tcpHandle].tcpResponseCb(tcpHandle, &tcpRespData[tcpHandle]);
    }

    /* Ignore EOT */
    (*respStrPtr)++;

    /* Free memory if allocated */
    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);

    /* Return parse status */
    return writeStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Image setting day night config parser
 * @param   respStrPtr
 * @param   tcpHandle
 * @return
 */
static BOOL dayNightCnfgParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle)
{
    UINT64 parseValue;
    BOOL   writeStatus = FAIL;

    /* Allocate memory for image setting param */
    IMAGE_CAPABILITY_INFO_t *pImageCapsInfo = malloc(sizeof(IMAGE_CAPABILITY_INFO_t));
    if (NULL == pImageCapsInfo)
    {
        /* Fail to alloc memory */
        return FAIL;
    }

    /* Free memory if already allocated */
    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);
    tcpRespData[tcpHandle].ciCfgData = pImageCapsInfo;

    /* Reset capability info before using it */
    memset(pImageCapsInfo, 0, sizeof(IMAGE_CAPABILITY_INFO_t));
    while (**respStrPtr != EOT)
    {
        /* Search for start of index */
        if (**respStrPtr != SOI)
        {
            continue;
        }

        /* Increment for SOI */
        (*respStrPtr)++;

        /* Get the index */
        if (ParseStringGetVal(respStrPtr, &parseValue, 1, FSP) == FAIL)
        {
            break;
        }

        /* Index should start from 0 or 1 */
        if (parseValue > 1)
        {
            EPRINT(CAMERA_INITIATION, "index max reached: [indexId=%d]", (UINT32)parseValue);
            break;
        }

        /* Search for end of index */
        while (**respStrPtr != EOI)
        {
            /* Get field string */
            if (ParseStringGetVal(respStrPtr, &parseValue, 1, FVS) == FAIL)
            {
                break;
            }

            switch (parseValue)
            {
                case CI_DAY_NIGHT_CNFG_LED_SENSITIVITY:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->irLedSensitivity.value = parseValue;
                }
                break;

                case CI_DAY_NIGHT_CNFG_LED_MODE:
                {
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                    pImageCapsInfo->irLed.mode = parseValue;
                }
                break;

                default:
                {
                    /* Skip other param from parsing */
                    writeStatus = ParseStringGetVal(respStrPtr, &parseValue, 1, FSP);
                }
                break;
            }

            /* Break process on failure */
            if (FAIL == writeStatus)
            {
                break;
            }
        }

        /* Break process on failure */
        if (FAIL == writeStatus)
        {
            break;
        }

        /* Ignore EOI */
        (*respStrPtr)++;

        /* Provide parsed data in callback for further processing */
        tcpRespData[tcpHandle].tcpResp = TCP_CLOSE_ON_SUCCESS;
        tcpClientInfo[tcpHandle].tcpResponseCb(tcpHandle, &tcpRespData[tcpHandle]);
    }

    /* Ignore EOT */
    (*respStrPtr)++;

    /* Free memory if allocated */
    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);

    /* Return parse status */
    return writeStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is used for parsing the command response.
 * @param   command id
 * @return  parser index
 */
static CI_CMD_PARSER_e getCmdParserIndex(UINT64 cmdId)
{
    switch (cmdId)
    {
        case CI_GET_IMG_SET_CAP:
            return CI_CMD_PARSER_IMAGE_SETTING;

        default:
            return CI_CMD_PARSER_MAX;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Image setting capability command's response parser
 * @param   respStrPtr
 * @param   tcpHandle
 * @return
 */
static BOOL imageSettingCapabilityCmdParser(CHARPTR *respStrPtr, TCP_HANDLE tcpHandle)
{
    UINT32 maxRange, paramCnfgId = 0;
    UINT32 paramType;
    UINT64 parseValue;
    BOOL   writeStatus = FAIL;
    CHAR   extraData[100];

    /* Allocate memory for image setting param */
    IMAGE_CAPABILITY_INFO_t *pImageCapsInfo = malloc(sizeof(IMAGE_CAPABILITY_INFO_t));
    if (NULL == pImageCapsInfo)
    {
        /* Fail to alloc memory */
        return FAIL;
    }

    /* Free memory if already allocated */
    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);
    tcpRespData[tcpHandle].ciCfgData = pImageCapsInfo;

    /* Reset capability info before using it */
    memset(pImageCapsInfo, 0, sizeof(IMAGE_CAPABILITY_INFO_t));

    /* Search for end of message */
    while (**respStrPtr != EOM)
    {
        /* Get field string */
        if (ParseStringGetVal(respStrPtr, &parseValue, 1, FVS) == FAIL)
        {
            break;
        }

        /* Store param type for reference */
        paramType = parseValue;
        switch (paramType)
        {
            case CI_IMAGE_SETTING_CAP_BRIGHTNESS:
            case CI_IMAGE_SETTING_CAP_CONTRAST:
            case CI_IMAGE_SETTING_CAP_SATURATION:
            case CI_IMAGE_SETTING_CAP_HUE:
            case CI_IMAGE_SETTING_CAP_SHARPNESS:
            case CI_IMAGE_SETTING_CAP_DWDR_STRENGTH:
            case CI_IMAGE_SETTING_CAP_EXPOSURE_RATIO:
            case CI_IMAGE_SETTING_CAP_EXPOSURE_GAIN:
            case CI_IMAGE_SETTING_CAP_EXPOSURE_IRIS:
            case CI_IMAGE_SETTING_CAP_NORMAL_LIGHT_LUMINANCE:
            case CI_IMAGE_SETTING_CAP_LED_SENSITIVITY:
            {
                UINT64                minValue, maxValue;
                IMG_CAP_RANGE_INFO_t *pParamRange = NULL;

                /* Parse range min value */
                writeStatus = ParseStringGetVal(respStrPtr, &minValue, 1, FVS);
                if (writeStatus == FAIL)
                {
                    break;
                }

                /* Parse range max value */
                writeStatus = ParseStringGetVal(respStrPtr, &maxValue, 1, FSP);
                if (writeStatus == FAIL)
                {
                    break;
                }

                /* Set value as per param */
                switch (paramType)
                {
                    case CI_IMAGE_SETTING_CAP_BRIGHTNESS:
                    {
                        maxRange = BRIGHTNESS_MAX;
                        paramCnfgId = IMG_SETTING_BRIGHTNESS;
                        pParamRange = &pImageCapsInfo->brightness;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_CONTRAST:
                    {
                        maxRange = CONTRAST_MAX;
                        paramCnfgId = IMG_SETTING_CONTRAST;
                        pParamRange = &pImageCapsInfo->contrast;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_SATURATION:
                    {
                        maxRange = SATURATION_MAX;
                        paramCnfgId = IMG_SETTING_SATURATION;
                        pParamRange = &pImageCapsInfo->saturation;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_HUE:
                    {
                        maxRange = HUE_MAX;
                        paramCnfgId = IMG_SETTING_HUE;
                        pParamRange = &pImageCapsInfo->hue;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_SHARPNESS:
                    {
                        maxRange = SHARPNESS_MAX;
                        paramCnfgId = IMG_SETTING_SHARPNESS;
                        pParamRange = &pImageCapsInfo->sharpness;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_DWDR_STRENGTH:
                    {
                        maxRange = (WDR_STRENGTH_MAX - WDR_STRENGTH_MIN);
                        paramCnfgId = IMG_SETTING_WDR_STRENGTH;
                        pParamRange = &pImageCapsInfo->wdrStrength;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_EXPOSURE_RATIO:
                    {
                        maxRange = (EXPOSURE_RATIO_MAX - EXPOSURE_RATIO_MIN);
                        paramCnfgId = IMG_SETTING_EXPOSURE_RATIO;
                        pParamRange = &pImageCapsInfo->exposureRatio;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_EXPOSURE_GAIN:
                    {
                        maxRange = (EXPOSURE_GAIN_MAX - EXPOSURE_GAIN_MIN);
                        paramCnfgId = IMG_SETTING_EXPOSURE_GAIN;
                        pParamRange = &pImageCapsInfo->exposureGain;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_EXPOSURE_IRIS:
                    {
                        maxRange = (EXPOSURE_IRIS_MAX - EXPOSURE_IRIS_MIN);
                        paramCnfgId = IMG_SETTING_EXPOSURE_IRIS;
                        pParamRange = &pImageCapsInfo->exposureIris;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_NORMAL_LIGHT_LUMINANCE:
                    {
                        maxRange = (NORMAL_LIGHT_LUMINANCE_MAX - NORMAL_LIGHT_LUMINANCE_MIN);
                        paramCnfgId = IMG_SETTING_NORMAL_LIGHT_LUMINANCE;
                        pParamRange = &pImageCapsInfo->normalLightLuminance;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_LED_SENSITIVITY:
                    {
                        maxRange = LED_SENSITIVITY_MAX;
                        paramCnfgId = IMG_SETTING_LED_SENSITIVITY;
                        pParamRange = &pImageCapsInfo->irLedSensitivity;
                    }
                    break;

                    default:
                    {
                        /* Nothing to do */
                    }
                    break;
                }

                /* Required param not found */
                if (NULL == pParamRange)
                {
                    break;
                }

                /* Validate min and max value */
                if (!IS_VALID_IMAGE_SETTING_RANGE(minValue, maxValue))
                {
                    break;
                }

                /* Store parsed values to param range and its capability */
                pParamRange->min = minValue;
                pParamRange->max = maxValue;
                pParamRange->step = GET_IMAGE_SETTING_STEP_VALUE(minValue, maxValue, maxRange);
                pImageCapsInfo->imagingCapability |= MX_ADD(paramCnfgId);
            }
            break;

            case CI_IMAGE_SETTING_CAP_WHITE_BALANCE:
            case CI_IMAGE_SETTING_CAP_WDR:
            case CI_IMAGE_SETTING_CAP_BACKLIGHT:
            case CI_IMAGE_SETTING_CAP_FLICKER:
            case CI_IMAGE_SETTING_CAP_NORMAL_LIGHT_GAIN:
            case CI_IMAGE_SETTING_CAP_LED_MODE:
            {
                UINT8                mode;
                UINT32               modeMask = 0;
                UINT64               parseMask = 0;
                IMG_CAP_MODE_INFO_t *pParamMode = NULL;

                /* Parse the supported modes in mask */
                writeStatus = ParseStringGetVal(respStrPtr, &parseMask, 1, FSP);
                if (writeStatus == FAIL)
                {
                    break;
                }

                /* Set value as per param */
                switch (paramType)
                {
                    case CI_IMAGE_SETTING_CAP_WHITE_BALANCE:
                    {
                        maxRange = WHITE_BALANCE_MODE_MAX;
                        paramCnfgId = IMG_SETTING_WHITE_BALANCE;
                        pParamMode = &pImageCapsInfo->whiteBalance;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_WDR:
                    {
                        maxRange = WDR_MODE_MAX;
                        paramCnfgId = IMG_SETTING_WDR_MODE;
                        pParamMode = &pImageCapsInfo->wdr;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_BACKLIGHT:
                    {
                        maxRange = BACKLIGHT_MODE_MAX;
                        paramCnfgId = IMG_SETTING_BACKLIGHT;
                        pParamMode = &pImageCapsInfo->backlightControl;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_FLICKER:
                    {
                        maxRange = FLICKER_MODE_MAX;
                        paramCnfgId = IMG_SETTING_FLICKER;
                        pParamMode = &pImageCapsInfo->flicker;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_NORMAL_LIGHT_GAIN:
                    {
                        maxRange = NORMAL_LIGHT_GAIN_MAX;
                        paramCnfgId = IMG_SETTING_NORMAL_LIGHT_GAIN;
                        pParamMode = &pImageCapsInfo->normalLightGain;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_LED_MODE:
                    {
                        maxRange = LED_MODE_MAX;
                        paramCnfgId = IMG_SETTING_LED_MODE;
                        pParamMode = &pImageCapsInfo->irLed;
                    }
                    break;

                    default:
                    {
                        /* Nothing to do */
                    }
                    break;
                }

                /* Required param not found */
                if (NULL == pParamMode)
                {
                    break;
                }

                /* Parse all the supported values of param */
                for (mode = 0; mode < maxRange; mode++)
                {
                    /* If it is valid value then set its bit */
                    if (GET_BIT(parseMask, mode))
                    {
                        modeMask |= MX_ADD(mode);
                    }
                }

                /* Do we get any valid mode? */
                if (modeMask)
                {
                    /* Store parsed values to param range and its capability */
                    pParamMode->modeSupported = modeMask;
                    pImageCapsInfo->imagingCapability |= MX_ADD(paramCnfgId);
                }
            }
            break;

            case CI_IMAGE_SETTING_CAP_EXPOSURE_RATIO_MODE:
            case CI_IMAGE_SETTING_CAP_EXPOSURE_MODE:
            {
                UINT64               parseMask = 0;
                IMG_CAP_MODE_INFO_t *pParamMode = NULL;

                /* Parse the supported modes in mask */
                writeStatus = ParseStringGetVal(respStrPtr, &parseMask, 1, FSP);
                if (writeStatus == FAIL)
                {
                    break;
                }

                /* There is a bug in camera code. It sends value 2 for checkbox value. Ideally, it should be 3 */
                if (parseMask == 0x02)
                {
                    parseMask = 0x03;
                }

                /* Verify checkbox mask */
                if (parseMask != 0x03)
                {
                    break;
                }

                /* Set value as per param */
                switch (paramType)
                {
                    case CI_IMAGE_SETTING_CAP_EXPOSURE_RATIO_MODE:
                    {
                        paramCnfgId = IMG_SETTING_EXPOSURE_RATIO_MODE;
                        pParamMode = &pImageCapsInfo->exposureRatioMode;
                    }
                    break;

                    case CI_IMAGE_SETTING_CAP_EXPOSURE_MODE:
                    {
                        paramCnfgId = IMG_SETTING_EXPOSURE_MODE;
                        pParamMode = &pImageCapsInfo->exposureMode;
                    }
                    break;

                    default:
                    {
                        /* Nothing to do */
                    }
                    break;
                }

                /* Store parsed values to param range and its capability */
                pParamMode->modeSupported = parseMask;
                pImageCapsInfo->imagingCapability |= MX_ADD(paramCnfgId);
            }
            break;

            default:
            {
                /* Skip other param from parsing */
                writeStatus = ParseStr(respStrPtr, FSP, extraData, sizeof(extraData));
            }
            break;
        }

        /* Break process on failure */
        if (FAIL == writeStatus)
        {
            break;
        }
    }

    /* Break process on failure */
    if (SUCCESS == writeStatus)
    {
        /* Ignore EOM */
        (*respStrPtr)++;

        /* Provide parsed data in callback for further processing */
        tcpRespData[tcpHandle].tcpResp = TCP_CLOSE_ON_SUCCESS;
        tcpClientInfo[tcpHandle].tcpResponseCb(tcpHandle, &tcpRespData[tcpHandle]);
    }

    /* Free memory if allocated */
    FREE_MEMORY(tcpRespData[tcpHandle].ciCfgData);

    /* Return parse status */
    return writeStatus;
}

// #################################################################################################
//  @END OF FILE
// #################################################################################################
