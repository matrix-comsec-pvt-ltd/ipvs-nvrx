#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H
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
//   Project      : DVR [Digital Video Recorder] - TI
//   Owner        :
//   File         : DataStructure.h
//   Description  :
/////////////////////////////////////////////////////////////////////////////

//#include <QObject>
#include "EnumFile.h"
#include <QRegExp>
#include <QDateTime>

#define DFLT_TCP_PORT           (8000)

typedef enum
{
    ANALOG_CAMERA,
    IP_CAMERA,
    AUTO_ADD_IP_CAMERA,
    MAX_CAMERA_TYPE

}CAMERA_TYPE_e;

typedef enum
{
    DATE_FORMAT_DDMMYYY,
    DATE_FORMAT_MMDDYYY,
    DATE_FORMAT_YYYYMMDD,
    DATE_FORMAT_WWWDDMMYYYY,
    MAX_DATE_FORMAT_TYPE

}DATE_FORMAT_e;

typedef enum
{
    TIME_FROMAT_12_HOURS,
    TIME_FORMAT_24_HOURS,
    MAX_TIME_FORMAT_TYPE

}TIME_FORMAT_e;

typedef enum
{
    REC_NATIVE_FORMAT,
    REC_BOTH_FORMAT,
    REC_AVI_FORMAT,
    MAX_REC_FORMAT

}RECORD_FORMAT_TYPE_e;

// Device Related Info structure
typedef struct
{
    quint8                      analogCams;                 // Total Analog Camera
    quint8                      ipCams;                     // Total Ip Camera
    quint8                      totalCams;                  // Total camera = Analog + IP
    quint8                      sensors;                    // Total Sensor of Device
    quint8                      alarms;                     // Total Alarms of Device
    quint8                      audioIn;                    // Audio Input support
    VIDEO_STANDARD_e            videoStd;                   // NTSC or Pal
    quint32                     mainEncodingCapacity;       // main Encoding capacity
    quint32                     subEncodingCapacity;        // sub Encoding capacity
    quint32                     maxAnalogCam;               // max supported analog camera
    quint32                     maxIpCam;                   // max supported ip camera
    quint8                      numOfHdd;                   // max HDD support
    quint8                      numOfLan;                   // max LAN support
    quint8                      maxMainAnalogResolution;    // max main Analog Resolution
    quint8                      maxSubAnalogResolution;     // max sub Analog Resolution
    USRS_GROUP_e                userGroupType;              // max userGroupType
    bool                        autoCloseRecFailAlert;      // auto Close Rec Fail Alert
    quint16                     videoPopUpDuration;         // videoPopupDuration
    quint16                     preVideoLossDuration;       // Pre Video Loss Duration
    bool                        startLiveView;              // Start Local Decoding
    MX_DEVICE_CONFLICT_TYPE_e   deviceConflictType;         // Local client compatibility with server
    quint32                     productVariant;             // Product Variant
    quint8                      recordFormatType;           // Recording Format

}DEV_TABLE_INFO_t;

// Camera Related Info structure
typedef struct
{
    bool            camStatus;          // cam state
    QString         camName;            // Cam Name
    OSD_POSITION_e  nameOsdPosition;    // Cam Name OSD position in window
    OSD_POSITION_e  statusOsdPosition;  // Cam Status OSD position in window
    CAMERA_TYPE_e   camType;            // Cam Type
    bool            dateTimerOverlay;
    OSD_POSITION_e  dateTimePosition;
    bool            textOverlay;
    QString         camText;
    OSD_POSITION_e  textPosition;
    quint8          cameraRights;

}DEV_CAM_INFO_t;

typedef struct
{
    char deviceName[MAX_DEVICE_NAME_SIZE];
    quint8 defChannel;
}CAM_INFO_t;

typedef struct _WINDOW_INFO_t
{
    CAM_INFO_t camInfo[MAX_WIN_SEQ_CAM];
    quint8 currentChannel;
    quint8 sequenceInterval;
    bool sequenceStatus;
    bool lastSequenceStatus;

    _WINDOW_INFO_t() : currentChannel(0), sequenceInterval(0), sequenceStatus(false), lastSequenceStatus(false)
    {

    }

}WINDOW_INFO_t;

typedef struct _DISPLAY_CONFIG_t
{
    // layout type
    LAYOUT_TYPE_e layoutId;
    // page number
    quint16 currPage;
    // sequence interval
    quint8 seqInterval;
    // sequence ON/OFF Status
    bool seqStatus;
    // selected window
    quint16 selectedWindow;
    // device name and cam name
    WINDOW_INFO_t windowInfo[MAX_CHANNEL_FOR_SEQ];

    _DISPLAY_CONFIG_t() : layoutId(MAX_LAYOUT), currPage(0), seqInterval(0), seqStatus(false), selectedWindow(0)
    {

    }

}DISPLAY_CONFIG_t;

class DevCommParam
{
public:
    REQ_MSG_ID_e        msgType;
    SET_COMMAND_e       cmdType;
    PWD_RST_CMD_e       pwdRstCmdType;
    DEVICE_REPLY_TYPE_e deviceStatus;
    QString             payload;
    quint8              *bytePayload;
    WIN_ID_TYPE_e       windowId;

    DevCommParam():msgType(MAX_REQ_MSG), cmdType(MAX_NET_COMMAND), pwdRstCmdType(PWD_RST_CMD_MAX),
        deviceStatus(CMD_REQUEST_NOT_PROCESSED), payload(""), bytePayload(NULL), windowId(MAX_WIN_ID)
    {
    }
};

// class which defines device audio parameters, like audio level
// and mute status
typedef struct
{
    // volume level
    quint8 level;
    // mute status
    AUDIO_MUTE_STATE_e muteStatus;

}AUDIO_CONFIG_t;

typedef struct
{
    quint16 startX;
    quint16 startY;
    quint16 width;
    quint16 height;
}PRIVACY_MASK_DATA_t;

typedef struct
{
    quint16 startRow;
    quint16 startCol;
    quint16 endRow;
    quint16 endCol;
    quint16 sensitivity;
}MOTION_DETECTION_WINDOWINFO_t;

typedef struct
{
    MOTION_DETECTION_WINDOWINFO_t windowInfo[MAX_MOTIONDETECTION_AREA];
    quint8 byteInfo[MAX_MOTION_BYTE];
    quint8 sensitivity;
    bool noMotionEventSupportF;
    bool isNoMotionEvent;
    quint16 noMotionDuration;
    MOTION_DETECTION_SUPPORT_TYPE_e motionSupportType;
}MOTION_DETECTION_CONFIG_t;

typedef struct
{
    quint16 startCol;
    quint16 startRow;
    quint16 endCol;
    quint16 endRow;
    quint8 index;
    quint8 sensitivity;
    bool drawFlag;
    bool isindividualBlock;
}MOTION_MASK_RECTANGLE_t;

typedef struct
{
    quint8 hue;
    quint8 saturation;
    quint8 brightness;
    quint8 contrast;
}IMAGE_APPEARENCE_DATA_t;

typedef struct
{
    quint8 cameraIndex;
    QDateTime startTime;
    QDateTime endTime;
    SYNC_PLAYBACK_EXPORT_STATUS_e exportStatus;
}CROP_AND_BACKUP_DATA_t;

typedef struct
{
    int winStartx;
    int winStarty;
    int winWidth;
    int winHeight;
}WIN_DIMENSION_INFO_t;

// class which defines textbox control required parameters,
class TextboxParam
{
public:
    QString textStr;
    QString labelStr;
    QString suffixStr;

    bool isEmailAddrType;
    // this flag is true if TextBox image start with bgtile centre
    bool isCentre;
    bool isForceCenter; /* Set box alignment center for no layer */

    // else left margin
    quint16 leftMargin;

    quint16 maxChar;
    quint16 minChar;

    bool isTotalBlankStrAllow;

    // this flag is true if textbox entry is numeric value
    bool isNumEntry;
    // and numeric value limitation
    quint32 minNumValue;
    quint32 maxNumValue;
    int extraNumValue;

    // validation at key in time
    // ********* must Specify when Num Entry**********//
    QRegExp validation;

    // final validation at "Done" key pressed
    QRegExp startCharVal;
    QRegExp middelCharVal;
    QRegExp endCharVal;


    TextboxParam():textStr(""),labelStr(""),suffixStr(""),
        isEmailAddrType(false),isCentre(true),isForceCenter(false),
        leftMargin(0),maxChar(0),minChar(0),isTotalBlankStrAllow(false),
        isNumEntry(false),minNumValue(0),maxNumValue(0),extraNumValue(-1),
        validation(QRegExp("")),startCharVal(QRegExp("")),
        middelCharVal(QRegExp("")),endCharVal(QRegExp(""))
    {
    }
};

typedef struct _SERVER_INFO_t
{
    QString ipAddress;
    quint16 tcpPort;

    _SERVER_INFO_t() : ipAddress(""), tcpPort(0)
    {

    }

}SERVER_INFO_t;

typedef struct
{
    QString sessionId;
    quint8 timeout;
    REQ_MSG_ID_e requestId;
    QString payload;
    quint8 *bytePayload;
    WIN_ID_TYPE_e windowId;

}REQ_INFO_t;

typedef struct _SESSION_INFO_t
{
    QString sessionId;
    quint8 timeout;

    _SESSION_INFO_t() : sessionId(""), timeout(0)
    {

    }
}SESSION_INFO_t;

typedef struct
{
    SERVER_INFO_t serverInfo;
    SESSION_INFO_t sessionInfo;
}SERVER_SESSION_INFO_t;

class StreamRequestParam
{
public:
    quint8 streamId;
    quint8 windowId;
    quint16 actualWindowId;
    quint8 channelId;
    QString deviceName;
    DISPLAY_TYPE_e displayType;
    STREAM_REQUEST_TYPE_e streamRequestType;
    LIVE_STREAM_TYPE_e liveStreamType;
    QString payload;
    bool audioStatus;
    qint64 timeStamp;

    StreamRequestParam():streamId(MAX_STREAM_SESSION),windowId(MAX_WINDOWS),
      actualWindowId(MAX_CHANNEL_FOR_SEQ),channelId(MAX_CAMERAS),
        deviceName(""),displayType(MAX_DISPLAY_TYPE),
        streamRequestType(MAX_STREAM_REQUEST_TYPE),
        liveStreamType(LIVE_STREAM_TYPE_MAIN),payload(""),audioStatus(false),timeStamp(0)
    {
    }

    void operator = (const StreamRequestParam &newParam)
    {
        streamId = newParam.streamId;
        windowId = newParam.windowId;
        actualWindowId = newParam.actualWindowId;
        channelId = newParam.channelId;
        deviceName = newParam.deviceName;
        displayType = newParam.displayType;
        streamRequestType = newParam.streamRequestType;
        liveStreamType = newParam.liveStreamType;
        payload = newParam.payload;
        audioStatus = newParam.audioStatus;
        timeStamp = newParam.timeStamp;
    }
};
#endif // DATASTRUCTURE_H
