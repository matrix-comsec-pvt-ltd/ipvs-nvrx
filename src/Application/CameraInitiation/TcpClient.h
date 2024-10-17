#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_
// #################################################################################################
//  FILE BRIEF
// #################################################################################################
/**
@file		TcpClient.h
@brief      This module provides TCP communication functionality for camera initiation.
*/
// #################################################################################################
//  @INCLUDES
// #################################################################################################
/* Application Includes */
#include "CameraDatabase.h"
#include "CameraEvent.h"
#include "CameraInitiation.h"
#include "Config.h"
#include "VideoParser.h"

// #################################################################################################
//  @DEFINES
// #################################################################################################
#define MAX_RELATIVE_URL_WIDTH          (512)
#define TCP_CONNECTION_TIMEOUT          (5)  // connection timeout in seconds
#define TCP_PUT_REBOOT_REQ_CONN_TIMEOUT (10)
#define TCP_FRAME_TIMEOUT               (10)   // 10 Second
#define MAX_FILE_NAME_LENGTH            (150)  // string of 150 char
#define MAX_TCP_QUEUE_SIZE              (100)

#define MAX_PROFILE_NAME_LENGTH         (100)
#define MAX_BITRATE_CTRL_NAME_LENGTH    (5)
#define MAX_DATE_FORMAT_NAME_LENGTH     (20)
#define MAX_TIMEFORMAT_NAME_LENGTH      (10)
#define MAX_OVERLAY_TEXT_LENGTH         (20)

#define MAX_MOTION_CELL_LENGTH          (400)
#define MAX_PRIVACY_WINDOW              (8)

#define MAX_TCP_REQUEST                 (MAX_CAMERA * 5)  // maximum number of acceptable request
#define INVALID_TCP_HANDLE              (0xFFFF)          // invalid TCP handle

// #################################################################################################
//  @DATA TYPES
// #################################################################################################
typedef UINT16 TCP_HANDLE;

typedef enum
{
    FRAME_WIDTH = 0,
    FRAME_HEIGHT,
    MAX_FRAME_DIMENSION
} FRAME_DIMENSION_e;

typedef enum
{
    TCP_CLOSE_ON_SUCCESS = 0,
    TCP_CLOSE_ON_ERROR,
    TCP_SUCCESS,
    TCP_ERROR,
    MAX_TCP_RESPONSE
} TCP_RESPONSE_e;

typedef enum
{
    CI_SET_CFG = 0,
    CI_GET_CFG = 1,
    CI_SRT_LV_STRM = 2,
    CI_STP_LV_STRM = 3,
    CI_INC_LV_AUD = 4,
    CI_EXC_LV_AUD = 5,
    CI_BITRATE_RES = 6,
    CI_SNP_SHT = 7,
    CI_OTHR_SUPP = 8,
    CI_ALARM_OUTPUT = 9,
    CI_MAX_PRIVACY_MASK = 10,
    CI_RPL_CMD = 11,
    CI_RPL_CFG = 12,
    CI_SET_CMD = 13,
    CI_SET_FOCUS = 14,
    CI_SET_ZOOM = 15,
    CI_SET_DATE_TIME = 16,
    CI_SRT_AUD_OUT = 17,
    CI_STP_AUD_OUT = 18,
    CI_GET_IMG_SET_CAP = 19,
    CI_SET_PANTILT = 20,
    CI_UPDATE_PRESET = 21,
    CI_CALL_PRESET = 22,
    MAX_TCP_REQUEST_TYPE
} TCP_REQUEST_e;

typedef enum
{
    CI_STREAM_PROFILE_CFG = 0,
    CI_APPEARANCE_CFG = 1,
    CI_OVERLAY_CFG = 2,
    CI_DAY_NIGHT_CFG = 3,
    CI_MOTION_DETECT_CFG = 7,
    CI_ADV_APPEARANCE_CFG = 14,
    MAX_CI_CFG_TABLE,
} CI_CFG_TABLE_e;

typedef enum
{
    CI_STREAM_CFG_NAME = 0,
    CI_STREAM_CFG_CODEC,
    CI_STREAM_CFG_RESL,
    CI_STREAM_CFG_FPS,
    CI_STREAM_CFG_BITCTRL,
    CI_STREAM_CFG_BITRATE,
    CI_STREAM_CFG_IMGQLTY,
    CI_STREAM_CFG_GOP,
    CI_STREAM_CFG_ADPTSTRM,
    CI_STREAM_CFG_BWOPT,
    CI_STREAM_CFG_AUD,
    CI_STREAM_CFG_MAX
} CI_STREAM_CFG_FIELD_e;

typedef enum
{
    CI_OVERLAY_CFG_ENABLE = 0,
    CI_OVERLAY_CFG_DATE,
    CI_OVERLAY_CFG_TIME,
    CI_OVERLAY_CFG_DISP,
    CI_OVERLAY_CFG_TXT_ENABLE,
    CI_OVERLAY_CFG_TXT0,
    CI_OVERLAY_CFG_TXT_POS0,
    CI_OVERLAY_CFG_ENBIMG,
    CI_OVERLAY_CFG_DISPIMG,
    CI_OVERLAY_CFG_ENBPRIMASK,
    CI_OVERLAY_CFG_ENBMASK1,
    CI_OVERLAY_CFG_STRTXMASK1,
    CI_OVERLAY_CFG_STRTYMASK1,
    CI_OVERLAY_CFG_WDTHMASK1,
    CI_OVERLAY_CFG_HTMASK1,
    CI_OVERLAY_CFG_ENBMASK2,
    CI_OVERLAY_CFG_STRTXMASK2,
    CI_OVERLAY_CFG_STRTYMASK2,
    CI_OVERLAY_CFG_WDTHMASK2,
    CI_OVERLAY_CFG_HTMASK2,
    CI_OVERLAY_CFG_ENBMASK3,
    CI_OVERLAY_CFG_STRTXMASK3,
    CI_OVERLAY_CFG_STRTYMASK3,
    CI_OVERLAY_CFG_WDTHMASK3,
    CI_OVERLAY_CFG_HTMASK3,
    CI_OVERLAY_CFG_ENBMASK4,
    CI_OVERLAY_CFG_STRTXMASK4,
    CI_OVERLAY_CFG_STRTYMASK4,
    CI_OVERLAY_CFG_WDTHMASK4,
    CI_OVERLAY_CFG_HTMASK4,
    CI_OVERLAY_CFG_ENBMASK5,
    CI_OVERLAY_CFG_STRTXMASK5,
    CI_OVERLAY_CFG_STRTYMASK5,
    CI_OVERLAY_CFG_WDTHMASK5,
    CI_OVERLAY_CFG_HTMASK5,
    CI_OVERLAY_CFG_ENBMASK6,
    CI_OVERLAY_CFG_STRTXMASK6,
    CI_OVERLAY_CFG_STRTYMASK6,
    CI_OVERLAY_CFG_WDTHMASK6,
    CI_OVERLAY_CFG_HTMASK6,
    CI_OVERLAY_CFG_ENBMASK7,
    CI_OVERLAY_CFG_STRTXMASK7,
    CI_OVERLAY_CFG_STRTYMASK7,
    CI_OVERLAY_CFG_WDTHMASK7,
    CI_OVERLAY_CFG_HTMASK7,
    CI_OVERLAY_CFG_ENBMASK8,
    CI_OVERLAY_CFG_STRTXMASK8,
    CI_OVERLAY_CFG_STRTYMASK8,
    CI_OVERLAY_CFG_WDTHMASK8,
    CI_OVERLAY_CFG_HTMASK8,
    CI_OVERLAY_CFG_TXT_ACTIVE1,
    CI_OVERLAY_CFG_TXT1,
    CI_OVERLAY_CFG_TXT_POS1,
    CI_OVERLAY_CFG_TXT_ACTIVE2,
    CI_OVERLAY_CFG_TXT2,
    CI_OVERLAY_CFG_TXT_POS2,
    CI_OVERLAY_CFG_TXT_ACTIVE3,
    CI_OVERLAY_CFG_TXT3,
    CI_OVERLAY_CFG_TXT_POS3,
    CI_OVERLAY_CFG_TXT_ACTIVE4,
    CI_OVERLAY_CFG_TXT4,
    CI_OVERLAY_CFG_TXT_POS4,
    CI_OVERLAY_CFG_TXT_ACTIVE5,
    CI_OVERLAY_CFG_TXT5,
    CI_OVERLAY_CFG_TXT_POS5,
    CI_OVERLAY_CFG_TXT_ACTIVE6,
    CI_OVERLAY_CFG_TXT6,
    CI_OVERLAY_CFG_TXT_POS6,
    CI_OVERLAY_CFG_MAX
} CI_OVERLAY_CFG_FIELD_e;

typedef enum
{
    CI_MOTION_CFG_ENB = 0,
    CI_MOTION_CFG_REDETECT,
    CI_MOTION_CFG_CELL,
    CI_MOTION_CFG_SENS,
    CI_MOTION_CFG_THRSHD,
    CI_MOTION_CFG_IS_NO_MOTION_EVNT,
    CI_MOTION_CFG_NO_MOTION_DURATION,
    CI_MOTION_CFG_MAX
} CI_MOTION_CFG_FIELD_e;

typedef enum
{
    CI_APPEARANCE_CNFG_BRIGHTNESS = 0,
    CI_APPEARANCE_CNFG_CONSTRAST,
    CI_APPEARANCE_CNFG_SATURATION,
    CI_APPEARANCE_CNFG_HUE,
    CI_APPEARANCE_CNFG_SHARPNESS,
    CI_APPEARANCE_CNFG_WHITE_BALANCE = 9,
    CI_APPEARANCE_CNFG_MAX,
} CI_APPEARANCE_CNFG_FIELD_e;

typedef enum
{
    CI_ADV_APPEARANCE_CNFG_WDR = 0,
    CI_ADV_APPEARANCE_CNFG_EXPOSURE_RATIO_MODE = 2,
    CI_ADV_APPEARANCE_CNFG_EXPOSURE_RATIO,
    CI_ADV_APPEARANCE_CNFG_EXPOSURE_FLICKER,
    CI_ADV_APPEARANCE_CNFG_EXPOSURE_MODE,
    CI_ADV_APPEARANCE_CNFG_EXPOSURE_GAIN = 7,
    CI_ADV_APPEARANCE_CNFG_EXPOSURE_IRIS,
    CI_ADV_APPEARANCE_CNFG_NORMAL_LIGHT_GAIN,
    CI_ADV_APPEARANCE_CNFG_NORMAL_LIGHT_LUMINANCE,
    CI_ADV_APPEARANCE_CNFG_BACKLIGHT = 17,
    CI_ADV_APPEARANCE_CNFG_DWDR_STRENGTH,
    CI_ADV_APPEARANCE_CNFG_MAX,
} CI_ADV_APPEARANCE_CNFG_FIELD_e;

typedef enum
{
    CI_DAY_NIGHT_CNFG_LED_SENSITIVITY = 0,
    CI_DAY_NIGHT_CNFG_LED_MODE,
    CI_DAY_NIGHT_CNFG_MAX,
} CI_DAY_NIGHT_CNFG_FIELD_e;

// config table 0 stream profile
typedef struct
{
    CHAR  name[MAX_PROFILE_NAME_LENGTH + 1];
    CHAR  codec[MAX_ENCODER_NAME_LEN];
    CHAR  resolution[MAX_RESOLUTION_NAME_LEN];
    INT16 fps;
    CHAR  bitRateCtl[MAX_BITRATE_CTRL_NAME_LENGTH];
    INT32 bitRate;
    UINT8 imageQuality;
    INT16 gop;
    UINT8 adaptiveStream;
    UINT8 bandWidthOpt;
    UINT8 audio;
} CI_STREAM_PROFILE_CONFIG_t;

typedef struct
{
    UINT8  maskEnable;
    UINT32 startXPoint;
    UINT32 startYPoint;
    UINT32 width;
    UINT32 height;
} CI_PRIVACY_MASK_CONFIG_t;

// config table 2 stream profile
typedef struct
{
    UINT8                    enableDateTime;
    CHAR                     dateFormate[MAX_DATE_FORMAT_NAME_LENGTH];
    CHAR                     timeFormate[MAX_TIMEFORMAT_NAME_LENGTH];
    OSD_POS_e                display;
    UINT8                    enableText;
    CHAR                     text[TEXT_OVERLAY_MAX][MAX_OVERLAY_TEXT_LENGTH];
    OSD_POS_e                channelNamePos[TEXT_OVERLAY_MAX];
    UINT8                    textOverlayMax;
    UINT8                    enablePrivacyMask;
    CI_PRIVACY_MASK_CONFIG_t privacyMaskArea[MAX_PRIVACY_WINDOW];
} CI_OVERLAY_CONFIG_t;

// request data
typedef struct
{
    UINT8          camIndex;
    TCP_REQUEST_e  tcpRequest;
    CI_CFG_TABLE_e cfgTableNum;
    CHAR           commandString[MAX_CAMERA_URI_WIDTH];
} TCP_REQ_INFO_t;

// response data
typedef struct
{
    UINT8               camIndex;
    TCP_RESPONSE_e      tcpResp;
    CI_NET_CMD_STATUS_e ciCmdResp;
    CHARPTR             storagePtr;
    UINT32              storageLen;
    INT32               connFd;
    VOIDPTR             ciCfgData;
} TCP_DATA_INFO_t;

typedef struct
{
    UINT8              state;
    UINT8              channel;
    STREAM_TYPE_e      streamType;
    VOIDPTR            dataPtr;
    MEDIA_FRAME_INFO_t frameInfo;
} TCP_FRAME_INFO_t;

typedef void (*TCP_CALLBACK)(TCP_HANDLE tcpHandle, TCP_DATA_INFO_t *dataInfo);
typedef void (*TCP_STREAM_CALLBACK)(TCP_FRAME_INFO_t *tcpFrame);

// #################################################################################################
//  @EXTERN VARIABLES
// #################################################################################################
extern const CHARPTR CIReqHeader[];

// #################################################################################################
//  @PROTOTYPES
// #################################################################################################
//-------------------------------------------------------------------------------------------------
void InitTcpClient(void);
//-------------------------------------------------------------------------------------------------
void DeInitTcpClient(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartTcp(TCP_REQ_INFO_t *tcpInfo, VOIDPTR callback, TCP_HANDLE *handlePtr, VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
// #################################################################################################
// @END OF FILE
// #################################################################################################
#endif /* TCPCLIENT_H_ */
