#ifndef ENUMFILE_H
#define ENUMFILE_H

///////////////////////////////////////////////////////////////////////////
//
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : NVR (Network Video Recorder)
//   Owner        : Shruti Sahni
//   File         : EnumFile.h
//   Description  : This file contains the enums required by most of the
//                  classes and qml.
/////////////////////////////////////////////////////////////////////////////
#include <QObject>
#include <QMetaType>
#include <QScreen>
#include <QGuiApplication>
#include <QtMath>
#include "../nvrgui/CommonDef.h"
#include "DebugLog.h"
#include "../DecoderLib/include/DecDispLib.h"

#define MAX_SYNC_PB_SESSION                 16

#define ONE_KILOBYTE                        1024
#define ONE_MEGABYTE                        (ONE_KILOBYTE * ONE_KILOBYTE)
#define ONE_MILISEC                         1000
#define MAX_CNFG_USERS                      128 //support of 128 users are added
#define MAX_SYSTEM_LANGUAGE                 6   /* Including English */

#if defined(RK3568_NVRL)
#define MAX_STREAM_SESSION                  (72)
#define MAX_WINDOWS                         (36)
#elif defined(RK3588_NVRH)
#define MAX_STREAM_SESSION                  (128)
#define MAX_WINDOWS                         (64)
#else
#define MAX_STREAM_SESSION                  (72)
#define MAX_WINDOWS                         (64)
#endif
#define MAX_LAYOUTS                         (16)
#define MAX_WIN_SEQ_CAM                     (16)

#define INVALID_CAMERA_INDEX                255
#define INVALID_STREAM_INDEX                MAX_STREAM_SESSION

#define MAX_CAMERAS                         96
#define CAMERAS_MAX_V1                      64

#define MAX_DEV_SENSOR                      8
#define MAX_DEV_ALARM                       4
#define MAX_CAM_ALARM                       3
#define MAX_USB_STS                         2
#define MAX_AUDIO_IN                        16
#define MAX_PRESET_POS                      40
#define MAX_LOGICAL_VOLUME                  16

#define MAX_LOCAL_DEVICE                    1
#define MAX_REMOTE_DEVICES                  20
#define MAX_DEVICES                         (MAX_LOCAL_DEVICE + MAX_REMOTE_DEVICES)
#define MAX_DEVICE_NAME_SIZE                17
#define MAX_CHANNEL_FOR_SEQ                 (MAX_DEVICES * MAX_CAMERAS)

#define NVR_SMART_CODE                      "28573"
#define LOCAL_DEVICE_NAME                   "Matrix-NVR"
#define DEFAULT_LOGIN_USER                  "local"

#define NORMAL_FONT_COLOR                   "#c8c8c8"
#define HIGHLITED_FONT_COLOR                "#528dc9"
#define SHADOW_FONT_COLOR                   "#000000"
#define SUFFIX_FONT_COLOR                   "#828182"
#define DISABLE_FONT_COLOR                  "#606060"
#define TRANSPARENTKEY_COLOR                "#808080"
#define WINDOW_GRID_COLOR                   "#555555"
#define SELECTED_WINDOW_GRID_COLOR          "#3a8af7"
#define NORMAL_BKG_COLOR                    "#252525"
#define CLICKED_BKG_COLOR                   "#1a1a1a"
#define PROCESS_BKG_COLOR                   "#06030e"
#define BORDER_1_COLOR                      "#404040"
#define BORDER_2_COLOR                      "#050505"
#define MASK_RECT_COLOR                     "#091008"
#define WINDOW_GRID_COLOR                   "#555555"
#define SELECTED_WINDOW_GRID_COLOR          "#3a8af7"
#define CALENDAR_FONT_COLOR                 "#8a8989"
#define SCHEDULE_BAR_COLOR                  "#3C5977"
#define POPUP_STATUS_LINE_COLOR             "#d3b68d"
#define MOUSE_HOWER_COLOR                   "#65affa"

#define TIME_BAR_COLOR                      "#696868"
#define TIME_BAR_FONT_COLOR                 "#818181"

#define DIFF_BTWN_BRKT_COLOR                "#596678"

#define GREEN_COLOR                         "#0c822d"
#define RED_COLOR                           "#990303"
#define YELOW_COLOR                         "#c39405"
#define BLUE_COLOR                          "#528dc9"
#define WHITE_COLOR                         "#c8c8c8"
#define QUICK_BACKUP_STATUS_COLOR           "#88aabf"
#define TIME_BAR_SLOT                       "#b3b3b3"

#define SOM     0x01        // start of message
#define EOM     0x04        // end of message
#define SOT     0x02        // start of table
#define EOT     0x03        // end of table
#define SOI     0x1C        // start of index
#define EOI     0x1D        // end of index
#define FSP     0x1E        // field separator
#define FVS     0x1F        // field value separator

#define SCREEN_HEIGHT       (ApplController::getHeightOfScreen())
#define SCREEN_WIDTH        (ApplController::getWidthOfScreen())

#define SCALE_WIDTH(controlWidth)     (qCeil(((controlWidth) * SCREEN_WIDTH)/1920))
#define SCALE_HEIGHT(controlHeight)   (qCeil(((controlHeight) * SCREEN_HEIGHT)/1080))
#define SCALE_FONT(fontSize)          (qCeil(((fontSize) * SCREEN_WIDTH)/1920))

#define SCALE_IMAGE(imgPixMap)  {if(imgPixMap.isNull() == false)\
                                    imgPixMap = imgPixMap.scaled(SCALE_WIDTH(imgPixMap.width()), SCALE_HEIGHT(imgPixMap.height()),\
                                                                 Qt::IgnoreAspectRatio, Qt::SmoothTransformation);}

#define SETTING_LEFT_PANEL_WIDTH            307
#define SETTING_LEFT_PANEL_HEIGHT           840
#define SETTING_RIGHT_PANEL_WIDTH           1065
#define SETTING_RIGHT_PANEL_HEIGHT          796

#define MANAGE_LEFT_PANEL_WIDTH             SCALE_WIDTH(307)
#define MANAGE_LEFT_PANEL_HEIGHT            SCALE_HEIGHT(442)
#define MANAGE_RIGHT_PANEL_WIDTH            SCALE_WIDTH(771)     //(705 + 35 + 31)
#define MANAGE_RIGHT_PANEL_HEIGHT           SCALE_HEIGHT(398)

#define PB_SEARCH_MAIN_RECT_WIDTH           SCALE_WIDTH(1145)
#define PB_SEARCH_MAIN_RECT_HEIGHT          SCALE_HEIGHT(657)
#define PB_SEARCH_HEADER_RECT_WIDTH         SCALE_WIDTH(290)
#define PB_SEARCH_HEADER_RECT_HEIGHT        SCALE_HEIGHT(45)

#define WIZARD_MAIN_RECT_WIDTH               SCALE_WIDTH(1145)
#define WIZARD_MAIN_RECT_HEIGHT              SCALE_HEIGHT(750)

#define DISP_SETTING_PAGE_WIDTH             1290 // 150 + 280 + 20 + 40 = 490

#define DISP_SETTING_PAGE_HEIGHT            895 //845 // 40 + 80 + 20 = 140

#define DISP_SETTING_PAGE_HEADING_WIDTH     290
#define DISP_SETTING_PAGE_HEADING_HEIGHT    45

#define NORMAL_FONT_FAMILY                  "Century Gothic"
#define WINDOW_FONT_FAMILY                  "LetterGothicStd"

#define EXTRA_SMALL_SUFFIX_FONT_SIZE        11
#define SMALL_SUFFIX_FONT_SIZE              13
#define SUFFIX_FONT_SIZE                    14
#define NORMAL_FONT_SIZE                    SCALE_FONT(15)
#define SUB_HEADING_FONT_SIZE               17
#define HEADING_FONT_SIZE                   22
#define MEDIUM_HEADING_FONT_SIZE            28
#define LARGE_HEADING_FONT_SIZE             34
#define RECT_RADIUS                         15

#define IMAGE_PATH                          ":/Images_Nvrx/"

#define MAX_PRIVACYMASK_AREA                8
#define MAX_MOTIONDETECTION_AREA            4
#define MAX_MOTION_BYTE                     198

#define MAX_PAL_ROW                         36
#define MAX_NTSC_ROW                        30

#define SUB_ACTIVITY_TYPE                   0
#define AUDIO_ON_PAGE_CHANGE                0

#define WINDOW_BORDER_WIDTH                 2

#define TOOLBAR_BUTTON_WIDTH                SCALE_WIDTH(75)
#define TOOLBAR_BUTTON_HEIGHT               SCALE_HEIGHT(50)

#define NO_VIDEO_LOSS                       0
#define VIDEO_LOSS                          1
#define MAX_VIDEO_LOSS_STATUS               2

#define CMD_DISK_OPERATION_TIMEOUT          50
#define CMD_LONG_TIMEOUT                    35
#define CMD_NORMAL_TIMEOUT                  7
#define CMD_SYNC_PB_FRAME_RECV_TIMEOUT      12
#define CMD_SHORT_TIMEOUT                   2

//when we give MAX_RECORDING_STORAGE_DRIVE in search parameter,
//server will search in current storage recording drive
#define MAX_RECORDING_STORAGE_DRIVE         3
#define MAX_EVENT_STRING_LENGTH             48
#define MAX_TIME_STRING_LENGTH              8
#define MAX_DATE_STRING_LENGTH              10

const QString imgTypePath [6] = {"Button_1.png",
                                 "Button_2.png",
                                 "Button_3.png",
                                 "Button_4.png",
                                 "Button_5.png",
                                 "Button_6.png"};

const QString asciiset1ValidationString = QString("[") + QString("a-zA-Z0-9\\-_.,():@!#$*+\\  \\[\\]/\\\\") + QString("]");
const QString asciiset1ValidationStringWithoutSpace = QString("[") + QString("a-zA-Z0-9\\-_.,():@!#$*+\\[\\]/\\\\") + QString("]");

#define DELETE_OBJ(obj)             if(obj != NULL){ delete obj; obj = NULL; }
#define IS_VALID_OBJ(obj)           ((obj != NULL) ? true : false)
#define INIT_OBJ(obj)               (obj = NULL);

#define INT_TO_QSTRING(value)       (QString("%1").arg(value))

#define MAX_CAMERA_NAME_LENGTH      16

#define MAX_POPUP_WINDOWS           (9)

#define QT_TRANSLATE_STR            "Matrix_NVR-X"
#define Multilang(msgString)        QApplication::translate(QT_TRANSLATE_STR, msgString)

//It is selected 129 by taking care of future expansion to 128 cameras
#define CLIENT_AUDIO_DECODER_ID     (129)

#define CAMERA_MASK_MAX                     2
#define CAMERA_BIT_WISE_MAX                 64
#define GET_CAMERA_MASK_IDX(camera)         ((camera)/CAMERA_BIT_WISE_MAX)
#define GET_CAMERA_BIT_IDX(camera)          ((camera)%CAMERA_BIT_WISE_MAX)
#define GET_CAMERA_MASK_BIT(mask, camera)   ((mask.bitMask[GET_CAMERA_MASK_IDX(camera)] >> GET_CAMERA_BIT_IDX(camera)) & 1ULL)
#define SET_CAMERA_MASK_BIT(mask, camera)   mask.bitMask[GET_CAMERA_MASK_IDX(camera)] |= ((quint64)1 << GET_CAMERA_BIT_IDX(camera));
#define CLR_CAMERA_MASK_BIT(mask, camera)   mask.bitMask[GET_CAMERA_MASK_IDX(camera)] &= ~((quint64)1 << GET_CAMERA_BIT_IDX(camera));
#define IS_ALL_CAMERA_MASK_BIT_CLR(mask)    ((mask.bitMask[0] == 0) && (mask.bitMask[1] == 0))

/////////////////////////////////////////////////////////////////////////////
//Class:  EnumClass
//Description:
//      This class contains the enums required by most of the classes and qml.
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
    quint32 softwareVersion;
    quint32 softwareRevision;
    quint32 commVersion;
    quint32 commRevision;
    quint32 responseTime;
    quint32 KLVTime;
    quint32 maxCameras;
    quint32 maxAnalogCameras;
    quint32 maxIpCameras;
    quint32 configuredAnalogCameras;
    quint32 configuredIpCameras;
    quint32 maxSensorInput;
    quint32 maxAlarmOutput;
    quint32 audioIn;
    quint32 audioOut;
    quint32 noOfHdd;
    quint32 noOfNdd;
    quint32 noOfLanPort;
    quint32 noOfVGA;
    quint32 hdmi1;
    quint32 hdmi2;
    quint32 CVBSMain;
    quint32 CVBSSpot;
    quint32 CVBSSpotAnalog;
    quint32 anlogPTZSupport;
    quint32 USBPort;
    quint32 maxMainAnalogResolution;
    quint32 maxSubAnalogResolution;
    quint32 videoStandard;
    quint32 maxMainEncodingCap;
    quint32 maxSubEncodingCap;
    quint32 diskCheckingCount;
    quint32 maxDisplayOutput;
    quint32 userGroup;
    quint32 passwordPolicyLockTime;
    quint32 passwordExpirationTime;
    quint32 productVariant;
    quint32 productSubRevision;

}NVR_DEVICE_INFO_t;

typedef struct
{
    quint64 bitMask[CAMERA_MASK_MAX];

}CAMERA_BIT_MASK_t ;

extern NVR_DEVICE_INFO_t    deviceRespInfo;
extern bool                 isMessageAlertLoaded;
extern bool                 isOnbootAuoCamSearchRunning;

typedef enum
{
    PREVIOUS_PAGE_NAVIGATION,
    NEXT_PAGE_NAVIGATION
}NAVIGATION_TYPE_e;

typedef enum
{
    CENTER_X_CENTER_Y,
    START_X_START_Y,
    START_X_CENTER_Y,
    START_X_END_Y,
    END_X_END_Y,
    END_X_CENTER_Y,
    END_X_START_Y,
    CENTER_X_END_Y,
    CENTER_X_START_Y,
    MAX_POINT_PARAM_TYPE
}POINT_PARAM_TYPE_e;

typedef enum
{
    DEVICE_COMM,
    AUDIO_SETTING,
    DISPLAY_SETTING,
    CONNECT_DEVICE,
    DISCONNECT_DEVICE,
    OTHER_LOGIN_ACTIVITY,
    LOCAL_LOGIN_ACTIVITY,
    TST_CAM_IMG_DELETE,
    MAX_ACTIVITY
}ACTIVITY_TYPE_e;

typedef enum
{
    READ_OTHER_LOGIN_PARAM = 0,
    WRITE_OTHER_LOGIN_PARAM,
    READ_WIZARD_PARAM,
    WRITE_WIZARD_PARAM,
    MAX_OTHER_LOGIN_SUB_ACT
}OTHER_LOGIN_SUB_ACTIVITY_e;

typedef enum
{
    READ_DISP_ACTIVITY = 0,
    WRITE_DISP_ACTIVITY,
    READ_DFLTSTYLE_ACTIVITY,
    SET_CVBS_CONFIG,
    GET_CVBS_CONFIG,
    MAX_DISP_ACTIVITY
}DISPLAY_ACTIVITY_e;

// audio control sub activity
typedef enum
{
    WRITE_AUDIO_ACTIVITY = 0,
    READ_AUDIO_ACTIVITY,
    MAX_AUDIO_ACTIVITY

}AUDIO_ACTIVITY_TYPE_e;

typedef enum
{
    FEATURE_STATE_NONE,
    FEATURE_STATE_VIDEO_ON,
    FEATURE_STATE_START_VIDEO,
    FEATURE_STATE_STOP_VIDEO,
    FEATURE_STATE_START_OLD_WINDOW,
    FEATURE_STATE_STOP_OLD_WINDOW
}FEATURE_STATE_e;

typedef enum
{
    VIDEO_POPUP_STATE_NONE,
    VIDEO_POPUP_BACKUP,
    VIDEO_POPUP_STATE_VIDEO_ON,
    VIDEO_POPUP_STATE_START_VIDEO_WAIT,
    VIDEO_POPUP_STATE_START_VIDEO,
    VIDEO_POPUP_STATE_STOP_VIDEO,
    VIDEO_POPUP_STATE_START_OLD_WINDOW,
    VIDEO_POPUP_STATE_STOP_OLD_WINDOW,
    VIDEO_POPUP_STATE_STOP_OLD_WINDOW_PROCESSING
}VIDEO_POPUP_FEATURE_STATE_e;

typedef enum
{
    IMAGE_APPEARENCE_FEATURE,
    PRIVACY_MASK_FEATURE,
    MOTION_DETECTION_FEATURE,
    ZOOM_FEATURE,
    MAX_CAMERA_FEATURE
}CAMERA_FEATURE_TYPE_e;

typedef enum
{
    USB_TYPE_MANUAL,
    USB_TYPE_SCHEDULED,
    MAX_USB_TYPE
}USB_TYPE_e;

typedef enum
{
    STATE_1 = 0,
    STATE_2,
    MAX_STATE_TYPE
}STATE_TYPE_e;

typedef enum
{
    ABOUT_US_BUTTON             = 0,
    LOG_BUTTON                  = 1,
    LIVE_VIEW_BUTTON            = 2,
    DISPLAY_MODE_BUTTON         = 3,
    ASYN_PLAYBACK_BUTTON        = 4,
    SYNC_PLAYBACK_BUTTON        = 5,
    EVENT_LOG_BUTTON            = 6,
    SETTINGS_BUTTON             = 7,
    MANAGE_BUTTON               = 8,
    SYSTEM_STATUS_BUTTON        = 9,
    AUDIO_CONTROL_BUTTON        = 10,
    SEQUENCE_BUTTON             = 11,
    QUICK_BACKUP                = 12,
    VIDEO_POPUP_BUTTON          = 13,
    WIZARD_BUTTON               = 14,
    STYLE_SELECT_BUTTON         = 15,
    COLLAPSE_BUTTON             = 16,
    BUZZER_CONTROL_BUTTON       = 17,
    LIVE_EVENT_BUTTON           = 18,
    USB_CONTROL_BUTTON          = 19,
    CPU_LOADS_BUTTON            = 20,
    MAX_TOOLBAR_BUTTON          = 21
}TOOLBAR_BUTTON_TYPE_e;

typedef enum
{
    VIDEO_TYPE_NONE = 0,
    VIDEO_TYPE_LIVESTREAM,
    VIDEO_TYPE_PLAYBACKSTREAM,
    VIDEO_TYPE_SYNCPLAYBAKSTREAM,
    VIDEO_TYPE_INSTANTPLAYBACKSTREAM,
    VIDEO_TYPE_LIVESTREAM_AWAITING
}VIDEO_STREAM_TYPE_e;

typedef enum
{
    VIDEO_STATUS_NONE = 0,
    VIDEO_STATUS_CONNECTING,
    VIDEO_STATUS_RUNNING,
    VIDEO_STATUS_VIDEOLOSS,
    VIDEO_STATUS_RETRY,
    VIDEO_STATUS_EVENTWAIT,
    VIDEO_STATUS_ERROR
}VIDEO_STATUS_TYPE_e;

typedef enum
{
    VIDEO_ERROR_NONE = 0,
    VIDEO_ERROR_NOUSERRIGHTS,
    VIDEO_ERROR_DISABLECAMERA,
    VIDEO_ERROR_CAMERADISCONNECTED,
    VIDEO_ERROR_OTHERERROR,
    VIDEO_ERROR_DEVICEDISCONNECTED,
    VIDEO_ERROR_VIDEOLOSS,
    VIDEO_ERROR_NO_DECODING_CAP,
    MAX_VIDEO_ERROR_TYPE
}VIDEO_ERROR_TYPE_e;

typedef enum
{
    LIVE_STREAM_TYPE_MAIN = 0,
    LIVE_STREAM_TYPE_SUB,
    MAX_LIVE_STREAM_TYPE
}LIVE_STREAM_TYPE_e;

typedef enum
{
    IP_ADDR_FAMILY_IPV4,
    IP_ADDR_FAMILY_IPV6,
    IP_ADDR_FAMILY_MAX,
}IP_ADDR_FAMILY_e;

typedef enum
{
    CAM_STATE_NONE = 0,
    CAM_STATE_CONNECTING,
    CAM_STATE_LIVE_STREAM,
    CAM_STATE_RETRY,
    CAM_STATE_ASSIGNED,
    MAX_CAMERA_STATE
}CAMERA_STATE_TYPE_e;

typedef enum
{
    TOP_NEIGHBOUR,
    BOTTOM_NEIGHBOUR,
    LEFT_NEIGHBOUR,
    RIGHT_NEIGHBOUR,
    MAX_NEIGHBOUR
}WIN_NEIGHBOUR_TYPE_e;

typedef enum
{
    IMAGE_TYPE_NORMAL,
    IMAGE_TYPE_MOUSE_HOVER,
    IMAGE_TYPE_CLICKED,
    IMAGE_TYPE_DISABLE,
    MAX_IMAGE_TYPE
}IMAGE_TYPE_e;

//Various Layouts supported
typedef enum
{
    ONE_X_ONE = 0,
    TWO_X_TWO,
    ONE_PLUS_FIVE,
    THREE_PLUS_FOUR,
    ONE_PLUS_SEVEN,
    THREE_X_THREE,
    TWO_PLUS_EIGHT,
    ONE_PLUS_NINE,
    ONE_PLUS_TWELVE,
    ONE_C_X_TWELVE,
    FOUR_PLUS_NINE,
    TWO_PLUS_TWELVE,
    FOUR_X_FOUR,
    FIVE_X_FIVE,
    SIX_X_SIX,
    EIGTH_X_EIGTH,
    ONE_X_ONE_PLAYBACK,
    TWO_X_TWO_PLAYBACK,
    THREE_X_THREE_PLAYBACK,
    FOUR_X_FOUR_PLAYBACK,
    MAX_LAYOUT
}LAYOUT_TYPE_e;

// display device enum
typedef enum
{
    MAIN_DISPLAY = 0,
    MAX_DISPLAY_TYPE
}DISPLAY_TYPE_e;

// audio mute state enum
typedef enum
{
    AUDIO_MUTE = 0,
    AUDIO_UNMUTE,
    MAX_AUDIO_MUTE_STATE

}AUDIO_MUTE_STATE_e;

// style type for different layouts
typedef enum
{
    STYLE_TYPE_1 = 0,
    STYLE_TYPE_2,
    STYLE_TYPE_3,
    STYLE_TYPE_4,
    STYLE_TYPE_5,
    MAX_STYLE_TYPE
}STYLE_TYPE_e;

// display setting param list enum
typedef enum
{
    DISPLAY_SUBACT = 0,
    DISPLAY_ID,
    DISPLAY_STYLE_ID,
    DISPLAY_DEFAULT_STATUS,
    MAX_DISPLAY_PARAM
}DISPLAY_ACT_PARAM_e;

typedef enum
{
    LOCAL_LOGIN_USERNAME = 0,
    LOCAL_LOGIN_PASSWORD,
    MAX_LOCAL_LOGIN_PARAM
}LOCAL_LOGIN_PARAM_e;

//Device Communication Activity's toCpp List Format
typedef enum
{
    MOTION_DETECTION_STS = 0,   //1
    VIEW_TAMPERED_STS,          //2
    CAM_CONN_STS,               //3
    CAM_SENSOR1_STS,            //4
    CAM_SENSOR2_STS,            //5
    CAM_SENSOR3_STS,            //6
    CAM_ALARM1_STS,             //7
    CAM_ALARM2_STS,             //8
    CAM_ALARM3_STS,             //9
    MANUAL_RECORDING_STS,       //10
    ALARM_RECORDING_STS,        //11
    SCHEDULE_RECORDING_STS,     //12
    COSEC_RECORDING_STS,        //13
    PTZ_TOUR_TYPE,              //14
    SENSOR_STS,                 //15
    ALARM_STS,                  //16
    DISK_STS,                   //17
    SCHEDULE_BACKUP_STS,        //18
    MANUAL_TRIGGER_STS,         //19
    MAINS_STS,                  //20
    BUZZER_STS,                 //21
    USB_STS,                    //22
    CAM_STREAM_STS,             //23
    TRIP_WIRE_STS,              //24
    OBJECT_INTRUSION_STS,       //25
    AUDIO_EXCEPTION_STS,        //26
    MISSING_OBJJECT_STS,        //27
    SUSPIOUS_OBJECT_STS,        //28
    LOITERING_OBJECT_STS,       //29
    OBJECT_COUNTING_STS,        //30
    ADAPTIVE_RECORDING_STS,
    MAX_PARAM_STS
}HEALTH_STS_PARAM_e;

typedef enum
{
    APPL_MOTION_DETECTION_STS = 0,  //1
    APPL_VIEW_TAMPERED_STS,         //2
    APPL_CAM_CONN_STS,              //3
    APPL_CAM_SENSOR1_STS,           //4
    APPL_CAM_SENSOR2_STS,           //5
    APPL_CAM_SENSOR3_STS,           //6
    APPL_CAM_ALARM1_STS,            //7
    APPL_CAM_ALARM2_STS,            //8
    APPL_CAM_ALARM3_STS,            //9
    APPL_MANUAL_RECORDING_STS,      //10
    APPL_ALARM_RECORDING_STS,       //11
    APPL_SCHEDULE_RECORDING_STS,    //12
    APPL_PTZ_TOUR_TYPE,             //13
    APPL_SENSOR_STS,                //14
    APPL_ALARM_STS,                 //15
    APPL_DISK_STS,                  //16
    APPL_SCHEDULE_BACKUP_STS,       //17
    APPL_MANUAL_TRIGGER_STS,        //18
    APPL_MAINS_STS,                 //19
    APPL_BUZZER_STS,                //20
    APPL_USB_STS,                   //21
    APPL_COSEC_RECORDING_STS,       //22
    APPL_STREAM_STS,                //23
    APPL_TRIP_WIRE_STS,             //24
    APPL_OBJECT_INTRUSION_STS,      //25
    APPL_AUDIO_EXCEPTION_STS,       //26
    APPL_MISSING_OBJECT,            //27
    APPL_SUSPIOUS_OBJECT,           //28
    APPL_LOTERING,                  //29
    APPL_OBJECT_COUNTING,           //30
    APPL_ADAPTIVE_RECORDING_STS,
    APPL_MAX_PARAM_STS
}APPL_HLTHSTS_PARAM_e;

// stream status
typedef enum
{
    STREAM_NORMAL = 0,
    STREAM_FILE_ERROR,
    STREAM_SYNC_START_INDICATOR,
    STREAM_PLAYBACK_PROCESS_ERROR,
    STREAM_RESERVED_4,
    STREAM_HDD_FORMAT,
    STREAM_CONFIG_CHANGE,
    STREAM_PLAYBACK_OVER,
    STREAM_PLAYBACK_SESSION_NOT_AVAIL,
    STREAM_PLAYBACK_CAM_PREVILAGE,
    MEDIA_REC_DRIVE_CONFIG_CHANGE,
    MEDIA_OTHER_ERROR,
    MAX_STREAM_STATUS
}STREAM_STATUS_e;

//enum for the device communication type
typedef enum
{
    BY_IP_ADDRESS = 0,
    BY_DDNS,
    BY_MATRIX_MAC,
    BY_MATRIX_HOSTNAME,
    MAX_CONN_TYPE
}CONNECTION_TYPE_e;

//enum for the device state
typedef enum
{
    CONNECTED = 0,
    DISCONNECTED,
    CONFLICT,
    LOGGED_OUT,
    DELETED,
    MAX_DEVICE_STATE
}DEVICE_STATE_TYPE_e;

typedef enum
{
    MSG_REQ_LOG = 0,    // 0
    MSG_REQ_POL,        // 1
    MSG_GET_CFG,        // 2
    MSG_SET_CFG,        // 3
    MSG_DEF_CFG,        // 4
    MSG_SET_CMD,        // 5
    MSG_REQ_EVT,        // 6
    MSG_PWD_RST,        // 7
    MAX_REQ_MSG         // 8
}REQ_MSG_ID_e;

//enum for the type of response by the device
typedef enum
{
    MSG_ACK_LOG = 0,    // 0
    MSG_RSP_POL,        // 1
    MSG_RPL_CFG,        // 2
    MSG_RPL_CMD,        // 3
    MSG_RCV_EVT,        // 4
    MSG_RPL_PWD,        // 5
    MAX_RES_MSG         // 6
}RES_MSG_ID_e;

//enum for the device response
typedef enum DEVICE_REPLY_TYPE_e
{
    CMD_SUCCESS = 0,                            // 00 : command success [generic]
    CMD_INVALID_MESSAGE,                        // 01 : error 0x01 [network manager]
    CMD_INVALID_SESSION,                        // 02 : error 0x02 [network manager]
    CMD_INVALID_SYNTAX,                         // 03 : error 0x03 [network manager]
    CMD_IP_BLOCKED,                             // 04 : IP blocked; connection failed [network manager]
    CMD_INVALID_CREDENTIAL,                     // 05 : invalid credential; login failed [network manager]
    CMD_USER_DISABLED,                          // 06 : NA
    CMD_USER_BLOCKED,                           // 07 : NA
    CMD_MULTILOGIN,                             // 08 : NA
    CMD_MAX_USER_SESSION,                       // 09 : maximum user session reached; login failed [network manager]
    CMD_RESOURCE_LIMIT,                         // 10 : resource not available [generic]
    CMD_NO_PRIVILEGE,                           // 11 : access denied; no privilege [network manager]
    CMD_INVALID_TABLE_ID,                       // 12 : invalid table id [configuration command]
    CMD_INVALID_INDEX_ID,                       // 13 : invalid index id [configuration command]
    CMD_INVALID_FIELD_ID,                       // 14 : invalid field id [configuration command]
    CMD_INVALID_INDEX_RANGE,                    // 15 : invalid index range [configuration command]
    CMD_INVALID_FIELD_RANGE,                    // 16 : invalid field range [configuration command]
    CMD_INVALID_FIELD_VALUE,                    // 17 : invalid field value [configuration command]
    CMD_MAX_STREAM_LIMIT,                       // 18 : maximum stream limit reached [media streamer]
    CMD_NO_PTZ_PROTOCOL,                        // 19 : NA
    CMD_NO_RECORD_FOUND,                        // 20 : no record found for search criteria [disk manager]
    CMD_SCHEDULE_TOUR_ON,                       // 21 : schedule tour is on; manual tour not allowed [PTZ tour]
    CMD_RECORDING_ON,                           // 22 : recording is on; manual tour not allowed [record manager / PTZ tour]
    CMD_MAX_BUFFER_LIMIT,                       // 23 : error 0x23 [generic]
    CMD_PROCESS_ERROR,                          // 24 : operation failed [generic]
    CMD_TESTING_ON,                             // 25 : testing already in process [TCP / SMTP / SMS / FTP]
    CMD_RESERVED_26,                            // 26 : Reserved (Used in DVR)
    CMD_NO_MODEM_FOUND,                         // 27 : no modem found [SMS]
    CMD_SIM_REGISTER_FAIL,                      // 28 : SIM registration failed [SMS]
    CMD_TESTING_FAIL,                           // 29 : testing failed [TCP / SMTP / SMS / FTP]
    CMD_SERVER_DISABLED,                        // 30 : server is disabled [TCP / SMTP / SMS / FTP]
    CMD_CONNECTIVITY_ERROR,                     // 31 : error in connection [TCP / SMTP / SMS / FTP]
    CMD_NO_WRITE_PERMISSION,                    // 32 : no write permission [FTP]
    CMD_INVALID_FILE_NAME,                      // 33 : invalid file name [system upgrade]
    CMD_CHANNEL_DISABLED,                       // 34 : camera disabled [camera interface]
    CMD_CHANNEL_BLOCKED,                        // 35 : NA
    CMD_SUB_STREAM_DISABLED,                    // 36 : NA
    CMD_BACKUP_IN_PROCESS,                      // 37 : backup in process; unable to format disk [disk manager / backup]
    CMD_ALARM_DISABLED,                         // 38 : alarm output disabled [IO]
    CMD_TOUR_NOT_SET,                           // 39 : manual PTZ tour not configured [PTZ tour]
    CMD_MAN_RECORD_DISABLED,                    // 40 : manual record is disabled [PTZ tour]
    CMD_NO_DISK_FOUND,                          // 41 : no HDD found [disk manager]
    CMD_NO_EVENT_FOUND,                         // 42 : no events found [event logger]
    CMD_ADMIN_CANT_BLOCKED,                     // 43 : NA
    CMD_ADDRESS_ALREADY_ASSIGNED,               // 44 : NA
    CMD_FILE_EXTRACT_ERROR,                     // 45 : file extraction error [system upgrade]
    CMD_FILE_MISSING,                           // 46 : upgrade file missing [system upgrade]
    CMD_REQUEST_IN_PROGRESS,                    // 47 : request already in process [generic]
    CMD_NO_EVENTS,                              // 48 : in response to long poll [network manager]
    CMD_EVENT_AVAILABLE,                        // 49 : in response to long poll [network manager]
    CMD_MORE_DATA,                              // 50 : more data / records available [record manager]
    CMD_MORE_EVENTS,                            // 51 : more events available [event logger]
    CMD_AUDIO_DISABLED,                         // 52 : audio disabled
    CMD_INVALID_UPDATE_MODE,                    // 53 : invalid mode of time update [date n time]
    CMD_STREAM_ALREADY_ON,                      // 54 : stream is already on [camera interface]
    CMD_PLAYBACK_STREAM_ON,                     // 55 : play-back already on [media streamer]
    CMD_MANUAL_RECORDING_ON,                    // 56 : manual recording already on [record manager]
    CMD_MANUAL_TOUR_ON,                         // 57 : manual tour already on [PTZ tour]
    CMD_MANUAL_RECORDING_OFF,                   // 58 : manual recording already off [PTZ tour]
    CMD_MANUAL_TOUR_OFF,                        // 59 : manual tour already off [PTZ tour]
    CMD_INVALID_SEQUENCE,                       // 60 : NA
    CMD_INVALID_DATA_LENGTH,                    // 61 : NA
    CMD_INVALID_FILE_LENGTH,                    // 62 : NA
    CMD_INVALID_FILE_SIZE,                      // 63 : NA
    CMD_UPGRADE_IN_PROCESS,                     // 64 : system upgrade is going on [system upgrade]
    CMD_FORMAT_IN_PROCESS,                      // 65 : format is going on [disk manager]
    CMD_DDNS_UPDATE_FAILED,                     // 66 : reserved for future use
    CMD_CAM_REQUEST_IN_PROCESS,                 // 67 : reserved for future use
    CMD_FEATURE_NOT_SUPPORTED,                  // 68 : reserved for future use
    CMD_CODEC_NOT_SUPPORTED,                    // 69 : reserved for future use
    CMD_CAM_REQUEST_FAILED,                     // 70 : reserved for future use
    CMD_CAM_DISCONNECTED,                       // 71 : reserved for future use
    CMD_NO_MANUAL_ACTION,                       // 72 : No manual trigger action configured
    CMD_FTP_INVALID_CREDENTIAL,                 // 73 : Invalid user name password for FTP
    CMD_BACKUP_NOT_IN_PROG,                     // 74 : Backup not in progress
    CMD_MAN_TRG_ALREADY_ON,                     // 75 : manual trigger already on
    CMD_MAN_TRG_ALREADY_OFF,                    // 76 : manual trigger already off
    CMD_CONFIG_RESTORE_IN_PROCESS,              // 77 : operation failed,Configuration restore in process
    CMD_ERROR_SUBMIT_ROUTE_TABLE,               // 78 : Destination address should not be in the Subnet of LAN1/LAN2
    CMD_DEST_ADDR_IS_NOT_NW_ADDR,               // 79 : Destination Address should be the network address
    CMD_UNABLE_TO_FORMAT,                       // 80 : Another disk is being formatted,unable to format
    CMD_LAN1_LAN2_SAME_SUBNET,                  // 81 : LAN1 and LAN2 should not be in same network
    CMD_SNAPSHOT_FAILED,                        // 82 : Snapshot failed
    CMD_SNAPSHOT_SERVER_DISABLED,               // 83 : snapshot failed FTP Server disabled
    CMD_SNAPSHOT_CONNECTION_ERROR,              // 84 : snapshot failed Error in connection
    CMD_SNAPSHOT_NO_WRITE_PERMISSION,           // 85 : snapshot failed No write permission
    CMD_CAM_PARAM_NOT_CONFIGURED,               // 86 : command parameter not configured
    CMD_ONVIF_CAM_CAPABILITY_ERROR,             // 87 : Unable to get parameters from ONVIF camera
    CMD_NON_CONFIGURABLE_PARAMETER,             // 88 : Failed-Non Configurable parameter
    CMD_FIRMWARE_NOT_FOUND,                     // 89 : Firmware Not Found
    CMD_DEVICE_FIRMWARE_UP_TO_DATE,             // 90 : Your device firmware is Up to Date
    CMD_SMS_ACC_EXPIRED,                        // 91 : SMS Account expired
    CMD_SMS_ACC_INSUFF_CREDITS,                 // 92 : SMS Account insufficient balance
    CMD_INVALID_MOBILE_NO,                      // 93 : Invalid mobile number
    CMD_IP_SUBNET_MISMATCH,                     // 94 : IP and Subnet mismatch
    CMD_PLAYBACK_PROCESS_ERROR,                 // 95 : Playback processing error
    CMD_INVALID_HOST_SIZE,                      // 96 : Invalid host size
    CMD_FILE_DOWNGRED_FAIL,                     // 97 : Firmware file not downgrade
    CMD_HOST_NAME_DUPLICATION,                  // 98 : Matrix dns hostname duplicate
    CMD_HOST_NAME_REG_FAIL,                     // 99 : Matrix dns hostname reg fail
    CMD_MAX_CAMERA_CONFIGURED,                  // 100 : Max Camera configured
    CMD_INVALID_USR_OR_PSWD,                    // 101 : invalid user name or password
    CMD_RESERVED_102,                           // 102 : Reserved
    CMD_REC_MEDIA_ERR,                          // 103 : recording media inaccessible
    CMD_REC_MEDIA_FULL,                         // 104 : recording media full
    CMD_RESERVED_105,                           // 105 : Reserved (Used in DVR)
    CMD_SAME_LAN_IP_ADDR,                       // 106 : for same lan 1 and lan 2 ip address
    CMD_BRND_MDL_MIS_MATCH,                     // 107 : Brand Model MisMatch
    CMD_IP_AND_GATEWAY_SAME_SUBNET,             // 108 : Ip and Default gateway in same subnet
    CMD_REC_DRIVE_CONFIG_CHANGES,               // 109 : hdd formatting while synchronous playback running
    CMD_GEN_CAM_STRM_PARAM_CAP_REQ,             // 110 : Generic camera stream param capability request
    CMD_NON_CONFIG_CAM_CAP_REQ,                 // 111 : non configured cam capability request
    CMD_SUB_STRM_NOT_SUPPORTED,                 // 112 : sub stream not supported
    CMD_MAX_CAM_CONFIGED,                       // 113 : Max Number of Cam Configed while auto search addition
    CMD_RESERVED_114,                           // 114 : Reserved (Used in DVR)
    CMD_AVI_SEARCH_NOT_ALLOWED,                 // 115 : Search in avi not allowed
    CMD_LOGIN_SESSION_DURATION_OVER,            // 116 : If Allowed Access Duration limit is reached
    CMD_INSTANT_PLAYBACK_FAILED,                // 117 : Unable to do Instant Playback
    CMD_MOTION_WINDOW_FAILED,                   // 118 : Unable to set configuration at camera end
    CMD_IP_ADDRESS_CHANGE_FAIL,                 // 119 : Unable to change the IP address
    CMD_CAM_REBOOT_FAILED,                      // 120 : Unable to reboot camera whose IP address change is requested
    CMD_NO_CAM_FOUND,                           // 121 : While searching if no camera is detected
    CMD_AUTO_SEARCH_COMPLETE,                   // 122 : auto Search compelete
    CMD_ACTIVE_TOUR_PAUSE,                      // 123 : preset tour pause
    CMD_AUTO_SEARCH_START,                      // 124 : indicate auto search process is started
    CMD_AUTO_CONFIG_RANGE_INVALID,              // 125 : indicate auto config ip address range is invalid

    CMD_RESET_PASSWORD,                         // 126 : password is not as per password policy
    CMD_PASSWORD_EXPIRE,                        // 127 : password is expire, now reset password
    CMD_USER_ACCOUNT_LOCK,                      // 128 : user is lock for lock duration.
    CMD_MIN_PASSWORD_CHAR_REQUIRED,             // 129 : minimum password characters required
    CMD_HIGH_PASSWORD_SEC_REQ,                  // 130 : high password security required
    CMD_MEDIUM_PASSWORD_SEC_REQ,                // 131 : medium password security required

    CMD_REQ_FAIL_CHNG_AUD_OUT_PRI = 133,        // 133 : Request failed. Change Audio Out priority
    CMD_AUDIO_CHANNEL_BUSY,                     // 134 : Audio channel busy
    CMD_NO_AUD_OUT_AVAILABLE,                   // 135 : No Audio Out port available
    CMD_AUD_SND_REQ_FAIL,                       // 136 : Audio sending request failed
    CMD_AUD_SND_STP_PRO_LCL_CLNT_REQ,           // 137 : Audio sending stopped. Processing local client	request
    CMD_RAID_TRANSFORM_FAIL,                    // 138 : Please check current HDD combination and try again.
    CMD_LOG_VOLUMN_MORE_THEN_8TB,               // 139 : RAID Creation failed as total logical volume size exceeded maximum allowed volume size.

    // Internal server codes
    CMD_DEVICE_EXIST = 141,                     // 141 : device exist
    CMD_DEV_CONNECTED,                          // 142 : device connected
    CMD_DEV_DISCONNECTED,                       // 143 : device disconnected
    CMD_DEV_CONFLICT,                           // 144 : device in conflict state
    CMD_SERVER_NOT_RESPONDING,                  // 145 : server not responding within response time
    CMD_STREAM_NORMAL,                          // 146 : live or playback stream status normal
    CMD_STREAM_FILE_ERROR,                      // 147 : live or playback stream status file error
    CMD_STREAM_HDD_FORMAT,                      // 148 : live or playback stream status hdd formatting
    CMD_STREAM_CONFIG_CHANGE,                   // 149 : live or playback stream status config change
    CMD_STREAM_PLAYBACK_OVER,                   // 150 : live or playback stream status playback over
    CMD_STREAM_VIDEO_LOSS,                      // 151 : live or playback stream status video loss
    CMD_STREAM_NO_VIDEO_LOSS,                   // 152 : live or playback stream status no video loss
    CMD_STREAM_STOPPED,                         // 153
    CMD_DUPLICATION_STOPPED,                    // 154
    CMD_DECODER_ERROR,                          // 155
    CMD_CONFIG_CHANGED,                         // 156
    CMD_INTERNAL_RESOURCE_LIMIT,                // 157
    CMD_INVALID_REQ_PARAM,                      // 158
    CMD_PLAYBACK_TIME,                          // 159
    CMD_STREAM_CONNECTING,                      // 160
    CMD_DEV_LOGGEDOUT,                          // 161
    CMD_DEV_DELETED,                            // 162
    CMD_CAM_INDX_NOT_VALID,                     // 163
    CMD_DISK_CLEANUP_REQUIRED,                  // 164 : login is sucessfull but diskcleanup is required
    CMD_REQUEST_NOT_PROCESSED,                  // 165
    CMD_STREAM_PB_SPEED,						// 166
    CMD_DECODER_CAPACITY_ERROR,                 // 167

    CMD_MAX_DEVICE_REPLY,                       // 168

    CMD_SMTP_SERVER_CONNECTION_ERROR = 1000,    // 1000 : Cannot connect to given Server & Port.
    CMD_SMTP_CONNECTION_REFUSED,				// 1001 : Connection Refused by SMTP server.
    CMD_SMTP_SERVER_UNAVAILABLE,                // 1002 : SMTP server not available at the moment
    CMD_SMTP_RECIPIENT_MAILBOX_FULL,			// 1003 : Recipient Mailbox full.
    CMD_SMTP_RECIPIENT_SERVER_NOT_RESPONDING,	// 1004 : Recipient server not responding.
    CMD_SMTP_MAIL_NOT_ACCEPTED,					// 1005 : Mail not accepted.
    CMD_SMTP_AUTHENTICATION_FAILED,				// 1006 : Authentication Failed.Verify Entered Details.
    CMD_SMTP_INVALID_EMAIL_ADDR,				// 1007 : Sender/Recipient mailbox address invalid or unavailable.
    CMD_SMTP_SERVER_STORAGE_FULL,				// 1008 : SMTP server storage limit exceeded.
    CMD_SMTP_TRANSACTION_FAILED,				// 1009 : Transaction Failed. Email is Spam/Blacklisted.

    CMD_INVALID_USERNAME = 1019,                // 1019 : Invalid username
    CMD_MISSING_VERIFICATION_DETAILS,           // 1020 : Verification details are not found
    CMD_MISMATCH_SECURITY_ANSWERS = 1022,       // 1022 : Wrong Answers
    CMD_MISMATCH_OTP,                           // 1023 : Mismatch OTP
    CMD_EMAIL_SERVICE_DISABLED,                 // 1024 : Email Service is not enabled
    CMD_PWD_RST_SESSION_NOT_AVAILABLE,          // 1025 : Password reset session limit exceeded
    CMD_PUSH_NOTIFICATION_NOT_ALLOWED,          // 1026 : Push Notifcation flag in User account is disabled
    CMD_PUSH_NOTIFICATION_DISABLED,             // 1027 : Push Notifcation is disabled for given user & fcm token

    CMD_PWD_RST_ALREADY_IN_PROGRESS = 1030,     // 1030 : Password reset already in progress
    CMD_SESSION_EXPIRED                         // 1031 : Session Expired

}DEVICE_REPLY_TYPE_e;

//enum for the device commands
typedef enum
{
    SRT_LV_STRM = 0,
    STP_LV_STRM,
    INC_LV_AUD,
    EXC_LV_AUD,
    CNG_LV_STRM,
    SRT_MAN_REC,
    STP_MAN_REC,
    SETPANTILT,
    SETZOOM,
    SETFOCUS,
    SETIRIS,
    CALLPRESET,
    SRT_MAN_PTZ_TOUR,
    STP_MAN_PTZ_TOUR,
    SEARCH_RCD_DATA,
    SEARCH_RCD_ALL_DATA,
    GET_PLY_STRM_ID,
    PLY_RCD_STRM,
    PAUSE_RCD_STRM,
    RSM_PLY_RCD_STRM,
    STEP_RCD_STRM,
    STP_RCD_STRM,
    CLR_PLY_STRM_ID,
    SEARCH_STR_EVT,
    ONLINE_USR,
    BLK_USR,
    VIEW_BLK_USR,
    UNBLK_USR,
    PHYSICAL_DISK_STS,
    LOGICAL_VOLUME_STS,
    HDDFORMAT,
    STOP_RAID,
    SRT_MAN_TRG,
    STP_MAN_TRG,
    GET_DATE_TIME,
    SET_DATE_TIME,
    LOGOUT,
    SHUTDOWN,
    RESTART,
    FAC_DEF_CFG,
    UPDT_DDNS,
    TST_MAIL,
    TST_FTP_CON,
    TST_TCP_CON,
    TST_SMS,
    USB_DISK_STS,
    USBFORMAT,
    USBUNPLUG,
    STP_BCKUP,
    ALRM_OUT,
    GET_MAC,
    REC_RGT_STS,
    BCK_RGT_STS,
    HEALTH_STS,
    BUZ_CTRL,
    SNP_SHT,
    BKUP_RCD,
    MODEM_STS,
    CHK_BAL,
    CNG_PWD,
    BRND_NAME,
    MDL_NAME,
    ENCDR_SUP,
    RES_SUP,
    IMG_RES_SUP,
    FR_SUP,
    QLT_SUP,
    OTHR_SUP,
    CNG_USER,
    UPDT_HOST_NM,
    CHK_PRIV,
    ADVANCE_STS,
    ADVANCE_STS_CAMERA,
    COSEC_VIDEO_POPUP,
    AUTO_SEARCH,
    ADD_CAMERAS,
    CNCL_AUTO_SEARCH,
    PLYBCK_SRCH_MNTH,
    PLYBCK_SRCH_DAY,
    PLYBCK_RCD,
    PAUSE_RCD,
    RSM_RCD,
    STP_RCD,
    CLR_RCD,
    SYNC_PLYBCK_RCD,
    SYNC_CLP_RCD,
    TST_CAM,
    GET_MOBILE_NUM,
    GET_USER_DETAIL,
    DISK_CLEANUP,
    GET_CAMERA_INFO,
    STRT_INSTANT_PLY,
    SEEK_INSTANT_PLY,
    STOP_INSTANT_PLY,
    PAUSE_INSTANT_PLY,
    RSM_INSTANT_PLY,
    GET_MAX_SUPP_PROF,
    GET_PROF_PARA,
    GET_BITRATE,
    GET_MOTION_WINDOW,
    GET_PRIVACY_MASK_WINDOW,
    ADV_CAM_SEARCH,
    CHANGE_IP_CAM_ADDRS,
    RESUME_PTZ_TOUR,
    SET_MOTION_WINDOW,
    GENERATE_FAILURE_REPORT,
    PTZ_TOUR_STATUS,
    MAX_ADD_CAM,
    TST_ND_CON,
    SET_PRIVACY_MASK_WINDOW,
    AUTO_CONFIGURE_CAMERA,
    GET_ACQ_LIST,
    CPU_LOAD,
    GET_USR_RIGHTS,
    GET_REG_CFG,
    AUTO_CFG_STATUS_RPRT,
    SET_PRESET,
    STRT_CLNT_AUDIO,
    STOP_CLNT_AUDIO,
    SND_AUDIO,
    STP_AUDIO,
    GET_CAM_INITIATED_LIST,
    ADD_CAM_INITIATED,
    RJCT_CAM_INITIATED,
    MAN_BACKUP,
    GET_LANGUAGE,
    GET_USER_LANGUAGE,
    SET_USER_LANGUAGE,
    GET_STATUS,
    SEARCH_FIRMWARE,
    START_UPGRADE,
    GET_DHCP_LEASE,
    GET_CAPABILITY,
    GET_PUSH_DEV_LIST,
    DEL_PUSH_DEV,
    TEST_EMAIL_ID,
    GET_PWD_RST_INFO,
    SET_PWD_RST_INFO,
    GET_MAN_BKP_LOC,
    VALIDATE_USER_CRED,
    MAX_NET_COMMAND,

    WIN_AUDIO,
    SYNC_PB_AUDIO_INCLD,
    SYNC_PLYBCK_SPEED,
    SYNC_STEP_FORWARD,
    SYNC_STEP_REVERSE,
    SYNC_PAUSE_BUFFER,
    SYNC_RSM_BUFFER,
    PAUSE_INSTANT_PLY_BUFFER,

    MAX_COMMAND

}SET_COMMAND_e;

typedef enum
{
    REQ_PWD_RST_SESSION = 0,
    CLR_PWD_RST_SESSION,
    GET_PWD_RST_OTP,
    VERIFY_PWD_RST_OTP,
    VERIFY_PWD_RST_QA,
    SET_NEW_PWD,
    PWD_RST_CMD_MAX
}PWD_RST_CMD_e;

// enumerator for event type
typedef enum
{
    LOG_ANY_EVENT = 0,
    LOG_CAMERA_EVENT,
    LOG_SENSOR_EVENT,
    LOG_ALARM_EVENT,
    LOG_SYSTEM_EVENT,
    LOG_STORAGE_EVENT,
    LOG_NETWORK_EVENT,
    LOG_OTHER_EVENT,
    LOG_USER_EVENT,
    LOG_COSEC_EVENT,
    LOG_MAX_EVENT_TYPE
}LOG_EVENT_TYPE_e;

// enumerator for event subtype
typedef enum
{
    // camera event subtypes
    LOG_NO_CAMERA_EVENT = 0,
    LOG_MOTION_DETECTION,
    LOG_VIEW_TEMPERING,
    LOG_CAMERA_SENSOR_1,
    LOG_CAMERA_SENSOR_2,
    LOG_CAMERA_SENSOR_3,
    LOG_CAMERA_ALARM_1,
    LOG_CAMERA_ALARM_2,
    LOG_CAMERA_ALARM_3,
    LOG_CONNECTIVITY,
    LOG_MANUAL_RECORDING,
    LOG_ALARM_RECORDING,
    LOG_SCHEDULE_RECORDING,
    LOG_PRESET_TOUR,
    LOG_SNAPSHOT_SCHEDULE,
    LOG_LINE_CROSSING,
    LOG_INTRUSION_DETECTION,
    LOG_AUDIO_EXCEPTION_DETECTION,
    LOG_VIDEO_POP_UP,
    LOG_PRESET_POSITION_CHANGE,
    LOG_MISSING_OBJECT,
    LOG_SUSPICIOUS_OBJECT,
    LOG_LOITERING,
    LOG_CAMERA_ONLINE,
    LOG_OBJECT_COUNTING,
    LOG_IMAGE_SETTING,
    LOG_STREAM_PARAM_COPY_TO_CAM,
    LOG_MOTION_DETECTION_COPY_TO_CAM,
    LOG_NO_MOTION_DETECTION,
    LOG_MAX_CAMERA_EVENT,

    // sensor event subtype
    LOG_NO_SENSOR_EVENT = 0,
    LOG_SENSOR_INPUT,
    LOG_MAX_SENSOR_EVENT,

    // alarm event subtype
    LOG_NO_ALARM_EVENT = 0,
    LOG_ALARM_OUTPUT,
    LOG_MAX_ALARM_EVENT,

    // system event subtype
    LOG_NO_SYSTEM_EVENT = 0,
    LOG_POWER_ON,
    LOG_MAINS_EVENT,
    LOG_UNAUTH_IP_ACCESS,
    LOG_RTC_UPDATE,
    LOG_DST_EVENT,
    LOG_SCHEDULE_BACKUP,
    LOG_MANUAL_BACKUP,
    LOG_SYSTEM_RESET,
    LOG_SHUTDOWN,
    LOG_LOGGER_ROLLOVER,
    LOG_RESTART,
    LOG_RECORDING_FAIL,
    LOG_AUTO_CFG_STS_REPORT,
    LOG_TIME_ZONE_UPDATE,
    LOG_TWO_WAY_AUDIO,
    LOG_RECORDING_RESTART,
    LOG_FIRMWARE_UPGRADE,
    LOG_MAX_SYSTEM_EVENT,

    // storage event subtype
    LOG_NO_STORAGE_EVENT = 0,
    LOG_HDD_STATUS,
    LOG_HDD_VOLUME_AT_INIT,
    LOG_HDD_VOLUME,
    LOG_HDD_CLEAN_UP,
    LOG_HDD_VOL_CLEAN_UP,
    LOG_MAX_STORAGE_EVENT,

    // network event subtype
    LOG_NO_NETWORK_EVENT = 0,
    LOG_ETHERNET_LINK,
    LOG_IP_ASSIGN,
    LOG_DDNS_IP_UPDATE,
    LOG_UPLOAD_IMAGE,
    LOG_EMAIL_NOTIFICATION,
    LOG_TCP_NOTIFICATION,
    LOG_SMS_NOTIFICATION,
    LOG_MAC_SERVER_UPDATE,
    LOG_MODEM_STATUS,
    LOG_P2P_STATUS,
    LOG_DHCP_SERVER_IP_ASSIGN,
    LOG_DHCP_SERVER_IP_EXPIRE,
    LOG_MAX_NETWORK_EVENT,

    // other event type
    LOG_NO_OTHER_EVENT = 0,
    LOG_UPGRADE_START,
    LOG_UPGARDE_STATUS_ERASE,
    LOG_UPGARDE_STATUS_WRITE,
    LOG_UPGARDE_STATUS_VERIFY,
    LOG_UPGRADE_RESULT,
    LOG_RESTORE_CONFIG_STRAT,
    LOG_RESTORE_CONFIG_STATUS_ERASE,
    LOG_RESTORE_CONFIG_STATUS_WRITE,
    LOG_RESTORE_CONFIG_STATUS_VERIFY,
    LOG_RESTORE_CONFIG_RESULT,
    LOG_BUZZER_STATUS,
    LOG_USB_STATUS,
    LOG_MANUAL_BACKUP_STATUS,
    LOG_MAX_OTHER_EVENT,

    // user events
    LOG_USER_NO_EVENT = 0,
    LOG_USER_SESSION,
    LOG_MANUAL_TRIGGER,
    LOG_CONFIG_CHANGE,
    LOG_SYS_CONFIGURATION,
    LOG_FIRMWARE,
    LOG_LOGIN_REQUEST,
    LOG_ALLOWED_ACCESS,
    LOG_SESSION_EXPIRE,
    LOG_PASSWORD_RESET,
    LOG_MAX_USER_EVENT,

    // COSEC events
    LOG_COSEC_NO_EVENT = 0,
    LOG_COSEC_RECORDING,
    LOG_COSEC_VIDEO_POP_UP,
    LOG_MAX_COSEC_EVENT

}LOG_EVENT_SUBTYPE_e;

// enumerator for event state
typedef enum
{
    EVENT_NORMAL = 0,
    EVENT_STOP = 0,
    EVENT_FAIL = 0,
    EVENT_REMOTE = 0,
    EVENT_LOGOUT = 0,
    EVENT_RESTORE = 0,
    EVENT_CREATING = 0,
    EVENT_INCOMPLETE_VOLUME = 0,
    EVENT_REGULAR = 0,
    EVENT_UP = 0,
    EVENT_PPPOE = 0,
    EVENT_DISCONNECT = 0,

    EVENT_ACTIVE = 1,
    EVENT_START = 1,
    EVENT_AUTO = 1,
    EVENT_ALERT = 1,
    EVENT_LOGIN = 1,
    EVENT_NO_DISK = 1,
    EVENT_DEFAULT = 1,
    EVENT_MISSING_DISK = 1,
    EVENT_FORMAT = 1,
    EVENT_BACKUP_FILE = 1,
    EVENT_CHANGE = 1,
    EVENT_DHCP = 1,
    EVENT_UPGRADE = 1,
    EVENT_DOWN = 1,
    EVENT_SUCCESS = 1,
    EVENT_CONNECT = 1,
    EVENT_RINGING = 1,

    EVENT_MANUAL = 2,
    EVENT_LOCAL = 2,
    EVENT_UNKNOWN = 2,
    EVENT_COMPLETE = 2,
    EVENT_MISSING_VOLUME = 2,
    EVENT_FULL = 2,

    EVENT_INCOMPLETE = 3,
    EVENT_LOW_MEMORY = 3,
    EVENT_PAUSE_TOUR = 3,

    EVENT_FAULT = 4,
    EVENT_RESUME_TOUR = 4

}LOG_EVENT_STATE_e;

typedef enum
{
    LV_EVT_DATE_TIME = 0,
    LV_EVT_TYPE,
    LV_EVT_SUB_TYPE,
    LV_EVT_DETAIL,
    LV_EVT_STATE,
    LV_EVT_ADV_DETAIL,
    MAX_LV_EVT_FIELD
}LV_EVT_FIELD_ORDER_e;

typedef enum
{
    CAMERA_INDEX,
    USER_NAME,
    POP_UP_TIME,
    USER_ID,
    MAX_POP_UP_DETAIL_INDEX
}POP_UP_EVT_DETAIL_e;

typedef enum
{
    TABLE_INDEX_NONE,                               // 0
    GENERAL_TABLE_INDEX,                            // 1
    DATETIME_TABLE_INDEX,                           // 2
    DST_TABLE_INDEX,                                // 3
    RS232_TABLE_INDEX,                              // 4
    LAN1_TABLE_INDEX,                               // 5
    LAN2_TABLE_INDEX,                               // 6
    IP_FILTER_APPLY_TABLE_INDEX,                    // 7
    IP_FILTER_TABLE_INDEX,                          // 8
    DDNS_TABLE_INDEX,                               // 9
    EMAIL_TABLE_INDEX,                              // 10
    FTP_TABLE_INDEX,                                // 11
    TCP_TABLE_INDEX,                                // 12
    FILE_ACCESS_SERICE_TABLE_INDEX,                 // 13
    HDD_MANAGMENT_TABLE_INDEX,                      // 14
    MATRIX_DNS_TABLE_INDEX,                         // 15
    USER_ACCOUNT_MANAGMENT_TABLE_INDEX,             // 16
    CAMERA_TABLE_INDEX,                             // 17
    STREAM_TABLE_INDEX,                             // 18
    ENABLE_SCH_REC_TABLE_INDEX,                     // 19
    SCH_REC_TABLE_INDEX,                            // 20
    ALARM_REC_TABLE_INDEX,                          // 21
    PRESET_POSITION_TABLE_INDEX,                    // 22
    MANUAL_PRESET_TOUR_TABLE_INDEX,                 // 23
    AUTO_PRESET_TOUR_TABLE_INDEX,                   // 24
    TOUR_SCHEDULE_TABLE_INDEX,                      // 25
    SYSTEM_SENSOR_INPUT_TABLE_INDEX,                // 26
    SYSTEM_ALARM_OUTPUT_INDEX,                      // 27
    UPLOAD_IMAGE_TABLE_INDEX,                       // 28
    STORAGE_MANAGMENT_TABLE_INDEX,                  // 29
    SCHEDULE_BACKUP_MANAGMENT_TABLE_INDEX,          // 30
    MANUAL_BACKUP_MANAGMENT_TABLE_INDEX,            // 31
    CAMERA_EVENT_ACTION_MANAGMENT_TABLE_INDEX,      // 32
    CAMERA_EVENT_ACTION_SCHDULE_TABLE_INDEX,        // 33
    SENSOR_EVENT_ACTION_MANAGMENT_TABLE_INDEX,      // 34
    SENSOR_EVENT_ACTION_SCHDULE_TABLE_INDEX,        // 35
    SYSTEM_EVENT_ACTION_MANAGMENT_TABLE_INDEX,      // 36
    COSEC_RECORDING_TABLE_INDEX,                    // 37
    CAMERA_ALARM_OUTPUT_TABLE_INDEX,                // 38
    DFT_ROUTING_TABLE_INDEX,                        // 39
    STATIC_ROUTING_TABLE_INDEX,                     // 40
    ACTIVE_BROADBAND_TABLE_INDEX,                   // 41
    BROADBAND_TABLE_INDEX,                          // 42
    SMS_TABLE_INDEX,                                // 43
    MANUAL_RECORDING_TABLE_INDEX,                   // 44
    NETWORK_DRIVE_MANAGMENT_TABLE_INDEX,            // 45
    IP_CAMERA_SETTING_TABLE_INDEX,                  // 46
    ANALOG_CAMERA_SETTINGS_TABLE_INDEX,             // 47
    PTZ_INTERFACE_SETTINGS_TABLE_INDEX,             // 48
    AUDIO_SETTING_TABLE_INDEX,                      // 49
    MONITORING_CLIENT_SETTING_TABLE_INDEX,          // 50
    NETWORK_DEVICE_SETTING_TABLE_INDEX,             // 51
    SNAPSHOT_SETTING_TABLE_INDEX,                   // 52
    SNAPSHOT_SCHEDULE_SETTING_TABLE_INDEX,          // 53
    PASSWORD_POLICY_SETTING_TABLE_INDEX,            // 54
    AUDIO_OUT_TABLE_INDEX,                          // 55
    P2P_TABLE_INDEX,                                // 56
    IMAGE_SETTINGS_TABLE_INDEX,                     // 57
    DHCP_SERVER_SETTINGS_TABLE_INDEX,               // 58
    FIRMWARE_MANAGEMNET_TABLE_INDEX,                // 59
    PUSH_NOTIFICATIONS_TABLE_INDEX,                 // 60
    PASSWORD_RECOVERY_TABLE_INDEX,                  // 61
    STORAGE_ALLOCATION_TABLE_INDEX,                 // 62
}CFG_TABLE_NO_e;

/************* PLAYBACK RELATED ENUMS****************/
// enum which specifies types of plaback stream states
typedef enum
{
    PB_PLAY_STATE,
    PB_STOP_STATE,
    PB_PAUSE_STATE,
    PB_STEP_STATE,
    PB_MAX_PLAYBACK_STATE
}PB_STREAM_STATE_e;

// direction type
typedef enum
{
    FORWARD_PLAY,
    BACKWARD_PLAY,
    MAX_PLAY_DIRECTION
}PB_DIRECTION_e;

typedef enum
{
    PB_SPEED_16S = 0,
    PB_SPEED_8S,
    PB_SPEED_4S,
    PB_SPEED_2S,
    PB_SPEED_NORMAL,
    PB_SPEED_2F,
    PB_SPEED_4F,
    PB_SPEED_8F,
    PB_SPEED_16F,
    MAX_PB_SPEED
}PB_SPEED_e;

typedef enum
{
    SYNC_PLAYBACK_NONE_STATE,
    SYNC_PLAYBACK_PLAY_STATE,
    SYNC_PLAYBACK_PAUSE_STATE,
    SYNC_PLAYBACK_STOP_STATE,
    SYNC_PLAYBACK_CROPANDBACKUP_STATE,
    SYNC_PLAYBACK_REVERSEPLAY_STATE,
    SYNC_PLAYBACK_STEP_STATE,
    SYNC_PLAYBACK_MAX_STATE
}SYNC_PLAYBACK_STATE_e;

typedef enum
{
    EXPORT_STATUS_NONE,
    EXPORT_STATUS_NOT_EXPORTED,
    EXPORT_STATUS_INCOMPLETE,
    EXPORT_STATUS_INPROGRESS,
    EXPORT_STATUS_EXPORTED,
    MAX_EXPORT_STATUS
}SYNC_PLAYBACK_EXPORT_STATUS_e;

typedef enum
{
    LIVE_ANALOG_REQUEST = 0,
    LIVE_STREAM_REQUEST,
    PLAYBACK_STREAM_REQUEST,
    SYNC_PLAYBACK_REQUEST,
    INSTANT_PLAYBACK_STREAM_REQUEST,
    CLIENT_AUDIO_REQUEST,
    MAX_STREAM_REQUEST_TYPE
}STREAM_REQUEST_TYPE_e;

typedef enum
{
    STREAM_NONE = 0,
    STREAM_RUN,
    STREAM_VL
}STREAM_STATE_e;

typedef enum
{
    CROSSED_MAXIMUM = 0,
    CROSSED_MINIMUM
}BUFFER_THRESHOLD_e;

typedef enum
{
    ICON_TYPE_3X3,
    ICON_TYPE_4X4,
    ICON_TYPE_5X5,
    ICON_TYPE_8X8
}WINDOW_ICON_TYPE_e;

typedef enum
{
    ADMIN = 0,      // administrator category
    OPERATOR,       // operator category
    VIEWER,         // viewer category
    MAX_USER_GROUP  // max number of user group type
}USRS_GROUP_e;

typedef enum
{
    POINT_METHOD = 0,
    BLOCK_METHOD
}MOTION_DETECTION_SUPPORT_TYPE_e;

typedef enum
{
    NO_STOP_NO_START,
    ONLY_SECOND_START,
    ONLY_FIRST_STOP,
    FIRST_STOP_SECOND_START,
    SECOND_REPLACE_FIRST
}REPLACESTREAMACTIVITY_TYPE_e;

typedef enum
{
    LIVE_CAM_LIST_TYPE,
    PLAYBACK_CAM_LIST_TYPE,
    MAX_CAM_LIST_TYPE
}CAM_LIST_TYPE_e;

typedef enum
{
    AUDIO_BITPOSITION,
    PTZ_BITPOSITION,
    PLAYBACK_BITPOSITION,
    MONITORING_BITPOSITION,
    VIDEO_POPUP_BITPOSITION,
    AUDIO_OUT_BITPOSITION,
    RESEV_BITS
}USER_ACC_MAN_RIGHTS_BITS_e;

typedef enum
{
    MAIN_DISPLAY_HDMI,
    MAX_MAIN_DISPLAY
}MAIN_DISPLAY_TYPE_e;

typedef enum
{
    MX_CAM_IDENTIFY,
    MX_CAM_ADDED,
    MX_CAM_UNIDENTIFY,
    MAX_MX_CAM_STATUS
}MX_CAM_STATUS_e;

typedef enum
{
    MX_DEVICE_CONFLICT_TYPE_SERVER_OLD,
    MX_DEVICE_CONFLICT_TYPE_SERVER_NEW,
    MAX_MX_DEVICE_CONFLICT_TYPE
}MX_DEVICE_CONFLICT_TYPE_e;

typedef enum
{
    NONE_PRIORITY,
    CLIENT_AUDIO_PRIORITY,
    CAMERA_AUDIO_PRIORITY,
    MAX_AUDIO_OUT_PRIORITY
}AUDIO_OUT_PRIORITY_e;

typedef enum
{
    CAPABILITY_CMD_ID_TEXT_OVERLAY = 0,
    CAPABILITY_CMD_ID_IMAGING_CAPABILITY,
    CAPABILITY_CMD_ID_ONVIF_MEDIA2_SUPPORT,
    CAPABILITY_CMD_ID_MAX
}CAPABILITY_CMD_ID_e;

typedef enum
{
    STATUS_CMD_ID_P2P = 0,
    STATUS_CMD_ID_SAMAS,
    STATUS_CMD_ID_MAX
}STATUS_CMD_ID_e;

typedef enum
{
    WIN_ID_LOGIN,
    WIN_ID_SETTINGS,
    WIN_ID_GEN_SETTINGS,
    WIN_ID_CAM_SETTINGS,
    WIN_ID_MXAUTO_ADD_CAM,
    WIN_ID_LIVE_VIEW_POPUP,
    MAX_WIN_ID
}WIN_ID_TYPE_e;

typedef enum
{
    LOG_VOL_NAME,
    LOG_TOTAL_SIZE,
    LOG_FREE_SIZE,
    LOG_STATUS,
    LOG_PERCENTAGE,
    LOG_DRIVE_TYPE,
    MAX_LOG_DRV_FEILDS
}LOG_DRV_FEILDS_e;

typedef enum
{
    PAGE_ID_MANAGE_PAGE,
    PAGE_ID_LIVE_VIEW,
    MAX_PAGE_ID
}USER_VALD_PAGE_ID_e;

#if defined(OEM_JCI)
#define NVRX_SUPPORT_START_VARIANT  HRIN_1208_18_SR
typedef enum
{
    NVR_NONE = 0,
    HRIN_1208_18_SR,
    HRIN_2808_18_SR,
    HRIN_4808_18_SR,
    HRIN_6408_18_SR,
    NVR_VARIANT_MAX
}NVR_VARIANT_e;
#else
// NVRX support added starting from below models
#define NVRX_SUPPORT_START_VARIANT  NVR0801X
typedef enum
{
    NVR_NONE = 0,
    HVR0408S,
    HVR0408P,
    HVR0824S,
    HVR0824P,
    HVR1624S,
    HVR1624P,
    NVR08S,
    NVR16S,
    NVR16H,
    NVR24P,
    NVR64S,
    NVR64P,
    NVR0801X,
    NVR1601X,
    NVR1602X,
    NVR3202X,
    NVR3204X,
    NVR6404X,
    NVR6408X,
    NVR9608X,
    NVR0801XP2,
    NVR1601XP2,
    NVR1602XP2,
    NVR3202XP2,
    NVR3204XP2,
    NVR6404XP2,
    NVR6408XP2,
    NVR0801XSP2,
    NVR1601XSP2,
    NVR0401XSP2,
    NVR9608XP2,
    NVR_VARIANT_MAX
}NVR_VARIANT_e;
#endif

const QString cmdString[MAX_COMMAND] =
{
    "SRT_LV_STRM",
    "STP_LV_STRM",
    "INC_LV_AUD",
    "EXC_LV_AUD",
    "CNG_LV_STRM",
    "SRT_MAN_REC",
    "STP_MAN_REC",
    "SETPANTILT",
    "SETZOOM",
    "SETFOCUS",
    "SETIRIS",
    "CALLPRESET",
    "SRT_MAN_PTZ_TOUR",
    "STP_MAN_PTZ_TOUR",
    "SEARCH_RCD_DATA",
    "SEARCH_RCD_ALL_DATA",
    "GET_PLY_STRM_ID",
    "PLY_RCD_STRM",
    "PAUSE_RCD_STRM",
    "RSM_PLY_RCD_STRM",
    "STEP_RCD_STRM",
    "STP_RCD_STRM",
    "CLR_PLY_STRM_ID",
    "SEARCH_STR_EVT",
    "ONLINE_USR",
    "BLK_USR",
    "VIEW_BLK_USR",
    "UNBLK_USR",
    "PHYSICAL_DISK_STS",
    "LOGICAL_VOLUME_STS",
    "HDDFORMAT",
    "CNCL_RAID_PRCS",
    "SRT_MAN_TRG",
    "STP_MAN_TRG",
    "GET_DATE_TIME",
    "SET_DATE_TIME",
    "LOGOUT",
    "SHUTDOWN",
    "RESTART",
    "FAC_DEF_CFG",
    "UPDT_DDNS",
    "TST_MAIL",
    "TST_FTP_CON",
    "TST_TCP_CON",
    "TST_SMS",
    "USB_DISK_STS",
    "USBFORMAT",
    "USBUNPLUG",
    "STP_BCKUP",
    "ALRM_OUT",
    "GET_MAC",
    "REC_RGT_STS",
    "BCK_RGT_STS",
    "HEALTH_STS",
    "BUZ_CTRL",
    "SNP_SHT",
    "BKUP_RCD",
    "CNCT_STS",
    "CHK_BAL",
    "CNG_PWD",
    "BRND_NAME",
    "MDL_NAME",
    "ENCDR_SUP",
    "RES_SUP",
    "IMG_RES_SUP",
    "FR_SUP",
    "QLT_SUP",
    "OTHR_SUP",
    "CNG_USER",
    "UPDT_HOST_NM",
    "CHK_PRIV",
    "ADVANCE_STS",
    "ADVANCE_STS_CAMERA",
    "COSEC_VIDEO_POPUP",
    "AUTO_SEARCH",
    "ADD_CAMERAS",
    "CNCL_SEARCH",
    "PLYBCK_SRCH_MNTH",        // commands for sync pb
    "PLYBCK_SRCH_DAY",
    "PLYBCK_RCD",
    "PAUSE_RCD",
    "RSM_RCD",
    "STP_RCD",
    "CLR_RCD",
    "SYNC_PLYBCK_RCD",
    "SYNC_CLP_RCD",
    "TST_CAM",
    "GET_MOBILE_NUM",
    "GET_USER_DETAIL",
    "DISK_CLEANUP",
    "GET_CAMERA_INFO",
    "STRT_INSTANT_PLY",
    "SEEK_INSTANT_PLY",
    "STOP_INSTANT_PLY",
    "PAUSE_INSTANT_PLY",
    "RSM_INSTANT_PLY",
    "GET_MAX_SUPP_PROF",
    "GET_PROF_PARA",
    "GET_BITRATE",
    "GET_MOTION_WINDOW",
    "GET_PRIVACY_MASK_WINDOW",
    "ADV_CAM_SEARCH",
    "CHANGE_IP_CAM_ADDRS",
    "RESUME_PTZ_TOUR",
    "SET_MOTION_WINDOW",
    "GENERATE_FAILURE_REPORT",
    "PTZ_TOUR_STATUS",
    "MAX_ADD_CAM",
    "TST_ND_CON",
    "SET_PRIVACY_MASK_WINDOW",
    "AUTO_CONFIGURE",
    "GET_ACQ_LIST",
    "CPU_LOAD",
    "GET_USR_RIGHTS",
    "GET_REG_CFG",
    "AUTO_CFG_STATUS_RPRT",
    "SET_PRESET",
    "STRT_CLNT_AUDIO",
    "STOP_CLNT_AUDIO",
    "SND_AUDIO",               //Used by device client to send audio
    "STP_AUDIO",               //Used by device client to stop audio
    "GET_CAM_INITIATED_LIST",
    "ADD_CAM_INITIATED",
    "RJCT_CAM_INITIATED",
    "MAN_BACKUP",
    "GET_LANGUAGE",
    "GET_USER_LANGUAGE",
    "SET_USER_LANGUAGE",
    "GET_STATUS",
    "SEARCH_FIRMWARE",
    "START_UPGRADE",
    "GET_DHCP_LEASE",
    "GET_CAPABILITY",
    "GET_PUSH_DEV_LIST",
    "DEL_PUSH_DEV",
    "TEST_EMAIL_ID",
    "GET_PWD_RST_INFO",
    "SET_PWD_RST_INFO",
    "GET_MAN_BKP_LOC",
    "VALIDATE_USER_CRED",
    "MAX_NET_COMMAND",

    "WIN_AUDIO",
    "SYNC_PB_AUDIO_INCLD",
    "SYNC_STEP_FORWARD",
    "SYNC_STEP_REVERSE",
    "SYNC_PAUSE_BUFFER",
    "SYNC_RSM_BUFFER",
};

const quint8 cmdTimeout[MAX_NET_COMMAND] =
{
    CMD_SYNC_PB_FRAME_RECV_TIMEOUT, // SRT_LV_STRM
    CMD_SHORT_TIMEOUT,              // STP_LV_STRM
    CMD_NORMAL_TIMEOUT,             // INC_LV_AUD
    CMD_NORMAL_TIMEOUT,             // EXC_LV_AUD
    CMD_SYNC_PB_FRAME_RECV_TIMEOUT, // CNG_LV_STRM
    CMD_NORMAL_TIMEOUT,             // SRT_MAN_REC
    CMD_NORMAL_TIMEOUT,             // STP_MAN_REC
    CMD_LONG_TIMEOUT,               // SETPANTILT
    CMD_LONG_TIMEOUT,               // SETZOOM
    CMD_LONG_TIMEOUT,               // SETFOCUS
    CMD_LONG_TIMEOUT,               // SETIRIS
    CMD_LONG_TIMEOUT,               // CALLPRESET
    CMD_NORMAL_TIMEOUT,             // SRT_MAN_PTZ_TOUR
    CMD_NORMAL_TIMEOUT,             // STP_MAN_PTZ_TOUR
    CMD_LONG_TIMEOUT,               // SEARCH_RCD_DATA
    CMD_LONG_TIMEOUT,               // SEARCH_RCD_ALL_DATA
    CMD_NORMAL_TIMEOUT,             // GET_PLY_STRM_ID
    CMD_NORMAL_TIMEOUT,             // PLY_RCD_STRM
    CMD_NORMAL_TIMEOUT,             // PAUSE_RCD_STRM
    CMD_NORMAL_TIMEOUT,             // RSM_PLY_RCD_STRM
    CMD_NORMAL_TIMEOUT,             // STEP_RCD_STRM
    CMD_NORMAL_TIMEOUT,             // STP_RCD_STRM
    CMD_NORMAL_TIMEOUT,             // CLR_PLY_STRM_ID
    CMD_LONG_TIMEOUT,               // SEARCH_STR_EVT
    CMD_NORMAL_TIMEOUT,             // ONLINE_USR
    CMD_NORMAL_TIMEOUT,             // BLK_USR
    CMD_NORMAL_TIMEOUT,             // VIEW_BLK_USR
    CMD_NORMAL_TIMEOUT,             // UNBLK_USR
    CMD_NORMAL_TIMEOUT,             // PHYSICAL_DISK_STS
    CMD_NORMAL_TIMEOUT,             // LOGICAL_VOLUME_STS
    CMD_NORMAL_TIMEOUT,             // HDDFORMAT
    CMD_NORMAL_TIMEOUT,             // CNCL_RAID_PRCS
    CMD_NORMAL_TIMEOUT,             // SRT_MAN_TRG
    CMD_NORMAL_TIMEOUT,             // STP_MAN_TRG
    CMD_NORMAL_TIMEOUT,             // GET_DATE_TIME    /* If change, update in Osd Thread also */
    CMD_NORMAL_TIMEOUT,             // SET_DATE_TIME
    CMD_NORMAL_TIMEOUT,             // LOGOUT
    CMD_NORMAL_TIMEOUT,             // SHUTDOWN
    CMD_NORMAL_TIMEOUT,             // RESTART
    CMD_NORMAL_TIMEOUT,             // FAC_DEF_CFG
    CMD_LONG_TIMEOUT,               // UPDT_DDNS
    CMD_LONG_TIMEOUT,               // TST_MAIL
    CMD_LONG_TIMEOUT,               // TST_FTP_CON
    CMD_LONG_TIMEOUT,               // TST_TCP_CON
    CMD_LONG_TIMEOUT,               // TST_SMS
    CMD_NORMAL_TIMEOUT,             // USB_DISK_STS
    CMD_NORMAL_TIMEOUT,             // USBFORMAT
    CMD_NORMAL_TIMEOUT,             // USBUNPLUG
    CMD_NORMAL_TIMEOUT,             // STP_BCKUP
    CMD_NORMAL_TIMEOUT,             // ALRM_OUT
    CMD_NORMAL_TIMEOUT,             // GET_MAC
    CMD_NORMAL_TIMEOUT,             // REC_RGT_STS
    CMD_NORMAL_TIMEOUT,             // BCK_RGT_STS
    CMD_NORMAL_TIMEOUT,             // HEALTH_STS
    CMD_NORMAL_TIMEOUT,             // BUZ_CTRL
    CMD_NORMAL_TIMEOUT,             // SNP_SHT
    CMD_LONG_TIMEOUT,               // BKUP_RCD
    CMD_NORMAL_TIMEOUT,             // CNCT_STS
    CMD_LONG_TIMEOUT,               // CHK_BAL
    CMD_NORMAL_TIMEOUT,             // CNG_PWD
    CMD_NORMAL_TIMEOUT,             // BRND_NAME
    CMD_NORMAL_TIMEOUT,             // MDL_NAME
    CMD_NORMAL_TIMEOUT,             // ENCDR_SUP
    CMD_NORMAL_TIMEOUT,             // RES_SUP
    CMD_NORMAL_TIMEOUT,             // IMG_RES_SUP
    CMD_NORMAL_TIMEOUT,             // FR_SUP
    CMD_NORMAL_TIMEOUT,             // QLT_SUP
    CMD_NORMAL_TIMEOUT,             // OTHR_SUP
    CMD_NORMAL_TIMEOUT,             // CNG_USER
    CMD_LONG_TIMEOUT,               // UPDT_HOST_NM
    CMD_NORMAL_TIMEOUT,             // CHK_PRIV
    CMD_NORMAL_TIMEOUT,             // ADVANCE_STS
    CMD_NORMAL_TIMEOUT,             // ADVANCE_STS_CAMERA
    CMD_NORMAL_TIMEOUT,             // COSEC_VIDEO_POPUP
    CMD_NORMAL_TIMEOUT,             // AUTO_SEARCH
    CMD_NORMAL_TIMEOUT,             // ADD_CAMERAS
    CMD_NORMAL_TIMEOUT,             // CNCL_AUTO_SEARCH
    CMD_DISK_OPERATION_TIMEOUT,     // PLYBCK_SRCH_MNTH
    CMD_DISK_OPERATION_TIMEOUT,     // PLYBCK_SRCH_DAY
    CMD_DISK_OPERATION_TIMEOUT,     // PLYBCK_RCD
    CMD_NORMAL_TIMEOUT,             // PAUSE_RCD
    CMD_NORMAL_TIMEOUT,             // RSM_RCD
    CMD_NORMAL_TIMEOUT,             // STP_RCD
    CMD_NORMAL_TIMEOUT,             // CLR_RCD
    CMD_DISK_OPERATION_TIMEOUT,     // SYNC_PLYBCK_RCD
    CMD_LONG_TIMEOUT,               // SYNC_CLP_RCD
    CMD_NORMAL_TIMEOUT,             // TST_CAM
    CMD_NORMAL_TIMEOUT,             // GET_MOBILE_NUM
    CMD_NORMAL_TIMEOUT,             // GET_USER_DETAIL
    CMD_NORMAL_TIMEOUT,             // DISK_CLEANUP
    CMD_NORMAL_TIMEOUT,             // GET_CAMERA_INFO
    CMD_NORMAL_TIMEOUT,             // STRT_INSTANT_PLY
    CMD_NORMAL_TIMEOUT,             // SEEK_INSTANT_PLY
    CMD_NORMAL_TIMEOUT,             // STOP_INSTANT_PLY
    CMD_NORMAL_TIMEOUT,             // PAUSE_INSTANT_PLY
    CMD_NORMAL_TIMEOUT,             // RSM_INSTANT_PLY
    CMD_NORMAL_TIMEOUT,             // GET_MAX_SUPP_PROF
    CMD_NORMAL_TIMEOUT,             // GET_PROF_PARA
    CMD_NORMAL_TIMEOUT,             // GET_BITRATE
    CMD_NORMAL_TIMEOUT,             // GET_MOTION_WINDOW
    CMD_NORMAL_TIMEOUT,             // GET_PRIVACY_MASK_WINDOW
    CMD_NORMAL_TIMEOUT,             // ADV_CAM_SEARCH
    CMD_NORMAL_TIMEOUT,             // CHANGE_IP_CAM_ADDRS
    CMD_NORMAL_TIMEOUT,             // RESUME_PTZ_TOUR
    CMD_LONG_TIMEOUT,               // SET_MOTION_WINDOW
    CMD_NORMAL_TIMEOUT,             // GENERATE_FAILURE_REPORT
    CMD_NORMAL_TIMEOUT,             // PTZ_TOUR_STATUS
    CMD_NORMAL_TIMEOUT,             // MAX_ADD_CAM
    CMD_LONG_TIMEOUT,               // TST_ND_CON
    CMD_LONG_TIMEOUT,               // SET_PRIVACY_MASK_WINDOW
    CMD_NORMAL_TIMEOUT,             // AUTO_CONFIGURE
    CMD_NORMAL_TIMEOUT,             // GET_ACQ_LIST
    CMD_NORMAL_TIMEOUT,             // CPU_LOAD
    CMD_NORMAL_TIMEOUT,             // GET_USR_RIGHTS
    CMD_NORMAL_TIMEOUT,             // GET_REG_CFG
    CMD_NORMAL_TIMEOUT,             // AUTO_CFG_STATUS_RPRT
    CMD_NORMAL_TIMEOUT,             // SET_PRESET
    CMD_SYNC_PB_FRAME_RECV_TIMEOUT, // STRT_CLNT_AUDIO
    CMD_NORMAL_TIMEOUT,             // STOP_CLNT_AUDIO
    CMD_NORMAL_TIMEOUT,             // SND_AUDIO
    CMD_NORMAL_TIMEOUT,             // STP_AUDIO
    CMD_NORMAL_TIMEOUT,             // GET_CAM_INITIATED_LIST
    CMD_NORMAL_TIMEOUT,             // ADD_CAM_INITIATED
    CMD_NORMAL_TIMEOUT,             // RJCT_CAM_INITIATED
    CMD_DISK_OPERATION_TIMEOUT,     // MAN_BACKUP
    CMD_NORMAL_TIMEOUT,             // GET_LANGUAGE
    CMD_NORMAL_TIMEOUT,             // GET_USER_LANGUAGE
    CMD_NORMAL_TIMEOUT,             // SET_USER_LANGUAGE
    CMD_NORMAL_TIMEOUT,             // GET_STATUS
    CMD_DISK_OPERATION_TIMEOUT,     // SEARCH_FIRMWARE
    CMD_DISK_OPERATION_TIMEOUT,     // START_UPGRAD
    CMD_NORMAL_TIMEOUT,             // GET_DHCP_LEASE
    CMD_NORMAL_TIMEOUT,             // GET_CAPABILITY
    CMD_NORMAL_TIMEOUT,             // GET_PUSH_DEV_LIST
    CMD_NORMAL_TIMEOUT,             // DEL_PUSH_DEV
    CMD_LONG_TIMEOUT,               // TEST_EMAIL_ID
    CMD_NORMAL_TIMEOUT,             // GET_PWD_RST_INFO
    CMD_NORMAL_TIMEOUT,             // SET_PWD_RST_INFO
    CMD_NORMAL_TIMEOUT,             // GET_MAN_BKP_LOC
    CMD_NORMAL_TIMEOUT,             // VALIDATE_USER_CRED
};

const QString deviceModelString[NVR_VARIANT_MAX] =
{
    #if defined(OEM_JCI)
    "",
    "HRIN_1208_18_SR",
    "HRIN_2808_18_SR",
    "HRIN_4808_18_SR",
    "HRIN_6408_18_SR",
    #else
    "",
    "HVR0408S",
    "HVR0408P",
    "HVR0824S",
    "HVR0824P",
    "HVR1624S",
    "HVR1624P",
    "NVR08S",
    "NVR16S",
    "NVR16H",
    "NVR24P",
    "NVR64S",
    "NVR64P",
    "NVR0801X",
    "NVR1601X",
    "NVR1602X",
    "NVR3202X",
    "NVR3204X",
    "NVR6404X",
    "NVR6408X",
    "NVR9608X",
    "NVR0801X P2",
    "NVR1601X P2",
    "NVR1602X P2",
    "NVR3202X P2",
    "NVR3204X P2",
    "NVR6404X P2",
    "NVR6408X P2",
    "NVR0801XS P2",
    "NVR1601XS P2",
    "NVR0401XS P2",
    "NVR9608X P2",
    #endif
};

const QString pwdRstCmdStr[PWD_RST_CMD_MAX] =
{
    "REQ_PWD_RST_SESSION",
    "CLR_PWD_RST_SESSION",
    "GET_PWD_RST_OTP",
    "VERIFY_PWD_RST_OTP",
    "VERIFY_PWD_RST_QA",
    "SET_NEW_PWD"
};

const quint8 pwdRstCmdTimeout[PWD_RST_CMD_MAX] =
{
    CMD_NORMAL_TIMEOUT,     // REQ_PWD_RST_SESSION
    CMD_NORMAL_TIMEOUT,     // CLR_PWD_RST_SESSION
    CMD_LONG_TIMEOUT,       // GET_PWD_RST_OTP
    CMD_NORMAL_TIMEOUT,     // VERIFY_PWD_RST_OTP
    CMD_NORMAL_TIMEOUT,     // VERIFY_PWD_RST_QA
    CMD_NORMAL_TIMEOUT,     // SET_NEW_PWD
};

#endif // ENUMFILE_H
