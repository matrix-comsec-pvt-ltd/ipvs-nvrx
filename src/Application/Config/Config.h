#if !defined CONFIGURATIONMODULE_H
#define CONFIGURATIONMODULE_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   Config.h
@brief  File contains Configuration data structures [structure, macro and enum] of NVR
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "ConfigComnDef.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
/** ******************************************************************************************* **/
/**                                  GENERAL SETTINGS                                           **/
/** ******************************************************************************************* **/
#define GENERAL_CONFIG_VERSION          (16)   //(15->16)  IPv6 Support
typedef struct
{
    CHAR                    deviceName[MAX_DEVICE_NAME_WIDTH];          // Device Name of the NVR module
    UINT8                   fileRecordDuration;                         // single file Record duration, in minutes
    UINT8                   autoLogoutDuration;                         // provides duration of auto logout feature, in minutes
    BOOL                    autoPowerOn;                                // Flag indicating whether to resume working after power is up
    UINT16                  httpPort;                                   // HTTP Server listen port
    UINT16                  tcpPort;                                    // TCP Server listen port (for control communication with PC client)
    RTP_PORT_RANGE_t        rtpPort;                                    // RTP port range used for media streams
    UINT16                  onvifPort;                                  // ONVIF port
    UINT8                   deviceId;                                   // Device id
    CAM_VIDEO_SYSTEM_e      videoSystemType;                            // NTSC / PAL
    BOOL                    integrateCosec;                             // Integrate with cosec
    DATE_FORMAT_e           dateFormat;                                 // format for date
    TIME_FORMAT_e           timeFormat;                                 // format for time
    RECORD_FORMAT_TYPE_e    recordFormatType;                           // format for time
    UINT8                   analogCamNo;                                // Analog Cam Number
    UINT8                   ipCamNo;                                    // IP Cam Number
	// Auto configure camera related parameter
    BOOL                    autoConfigureCameraFlag;                    // camrea auto configure flag
    BOOL                    retainIpAddresses;                          // Ip address retain flag
    CHAR                    autoConfigStartIp[IPV6_ADDR_LEN_MAX];       // auto config start ip
    CHAR                    autoConfigEndIp[IPV6_ADDR_LEN_MAX];         // auto config end ips
    BOOL                    retainDfltProfile;                          // retain profile default parameter
    CHAR                    videoEncoding[MAX_ENCODER_NAME_LEN];        // video encoding method
    CHAR                    resolution[MAX_RESOLUTION_NAME_LEN];        // image resolution
    UINT8                   framerate;                                  // frame rate
    UINT8                   quality;                                    // quality of stream
    BOOL                    enableAudio;                                // control to enable/disable audio
    BITRATE_MODE_e          bitrateMode;
    BITRATE_VALUE_e         bitrateValue;
    UINT8                   gop;
    CHAR                    videoEncodingSub[MAX_ENCODER_NAME_LEN];     // video encoding method
    CHAR                    resolutionSub[MAX_RESOLUTION_NAME_LEN];     // image resolution
    UINT8                   framerateSub;                               // frame rate
    UINT8                   qualitySub;                                 // quality of stream
    BOOL                    enableAudioSub;                             // control to enable/disable audio
    BITRATE_MODE_e          bitrateModeSub;
    BITRATE_VALUE_e         bitrateValueSub;
    UINT8                   gopSub;
    BOOL                    devInitWithServerF;                         // intigrate with samas Flag
    CHAR                    devInitServerIpAddr[IPV6_ADDR_LEN_MAX];     // samas Server Ip addr
    UINT16                  devInitServerPort;                          // samas Server Port
    BOOL                    autoRecordFailFlag;
    UINT16                  videoPopupDuration;
    UINT16                  preVideoLossDuration;
    CHAR                    userName[MAX_USERNAME_WIDTH];
    CHAR                    password[MAX_PASSWORD_WIDTH];
    BOOL                    startLiveView;                              // For Decoding in Local Client
    UINT16                  forwardedTcpPort;
    BOOL                    autoAddCamFlag;
    UINT16                  autoAddCamTcpPort;
    UINT8                   pollDuration;
    UINT8                   pollInterval;
    BOOL                    netAcceleration;

} GENERAL_CONFIG_t;

/** ******************************************************************************************* **/
/**                                DATE-TIME SETTINGS                                           **/
/** ******************************************************************************************* **/
#define DATE_TIME_CONFIG_VERSION        (5)//(3->4)  Auto Sync Time-zone(ONVIF) (4->5)setUTCtime flag
typedef struct
{
    UINT8                   timezone;                   // universal time zone
    DATE_TIME_UPDATE_MODE_e updateMode;                 // Mode of time update
    NTP_PARAMETER_V1_t      ntpV1;                      // NTP Parameters, <server and update time interval>
    BOOL                    autoUpdateRegional;         // Auto-update Regional
    NTP_PARAMETER_t         ntp;                        // NTP Parameters, <server and update time interval> // Note : UTF-8 change
    BOOL                    syncTimeZoneToOnvifCam;     // Auto Sync Time-zone(ONVIF)
    BOOL                    setUtcTime;                 // Set UTC Date time, if false then use local time
} DATE_TIME_CONFIG_t;

/** ******************************************************************************************* **/
/**                                    DST SETTINGS                                             **/
/** ******************************************************************************************* **/
#define DST_CONFIG_VERSION              (1)
typedef struct
{
    BOOL 				dst;            // DST function ON/OFF
    DST_CLOCK_t 		forwardClock;   // DST forward parameter
    DST_CLOCK_t 		reverseClock;   // DST reverse parameter
    TIME_HH_MM_t 		period;         // provides time duration to be changed

} DST_CONFIG_t;

/** ******************************************************************************************* **/
/**                                     LAN SETTINGS                                            **/
/** ******************************************************************************************* **/
#define LAN_CONFIG_VERSION              (5) // (4->5) IPv6 parameters added
typedef struct
{
    IP_ADDR_MODE_e      ipAddrMode;     // Ip addressing mode
    IPV4_LAN_CONFIG_t   ipv4;           // lan config for IPv4 addressing mode
    IPV6_LAN_CONFIG_t   ipv6;           // lan config for IPv6 addressing mode

} LAN_CONFIG_t;

/** ******************************************************************************************* **/
/**                                 MAC SERVER SETTINGS                                         **/
/** ******************************************************************************************* **/
#define MAC_SERVER_CONFIG_VERSION		(1)
typedef struct
{
    SERVER_CONNECT_MODE_e   connectMode;
    CHAR                    ip[IPV4_ADDR_LEN_MAX];
    CHAR                    name[MAX_MAC_SERVER_NAME_LEN];
    UINT16                  port;
    CHAR                    service[MAX_MAC_SERVICE_LEN];
    UINT16                  key;
    CHAR                    password[MAX_MAC_SERVER_PASSWORD_LEN];

} MAC_SERVER_CNFG_t;

/** ******************************************************************************************* **/
/**                                  IP FILTER SETTINGS                                         **/
/** ******************************************************************************************* **/
#define IP_FILTER_CONFIG_VERSION		(2) // (1->2) IPv6 support added
typedef struct
{
    BOOL                        ipFilter;               // control to Enable/Disable IP filter
    IP_ADDRESS_FILTER_MODE_e    mode;                   // one of the available filtering option
    IP_ADDRESS_RANGE_t          filter[MAX_IP_FILTER];  // specifies IP address range

} IP_FILTER_CONFIG_t;

/** ******************************************************************************************* **/
/**                                     DDNS SETTINGS                                           **/
/** ******************************************************************************************* **/
#define DDNS_CONFIG_VERSION				(2) //(1->2) UTF-8 mutlilanguage
typedef struct
{
    BOOL 				ddns;                                   // control to enable/disable DDNS service
	DDNS_SERVER_e 		server; // DDNS server
	CHAR 				usernameV1[MAX_DDNS_USERNAME_WIDTH_V1]; // username for authentication at DNS server
	CHAR 				passwordV1[MAX_DDNS_PASSWORD_WIDTH_V1]; // password for authentication at DNS server
	CHAR 				hostnameV1[MAX_DDNS_HOSTNAME_WIDTH_V1]; // hostname to be updated at DNS server
    UINT8 				updateDuration;                         // update duration for DNS, in minutes
    // new variables (UTF-8)
    CHAR 				username[MAX_DDNS_USERNAME_WIDTH];      // username for authentication at DNS server
    CHAR 				password[MAX_DDNS_PASSWORD_WIDTH];      // password for authentication at DNS server
    CHAR 				hostname[MAX_DDNS_HOSTNAME_WIDTH];      // hostname to be updated at DNS server
} DDNS_CONFIG_t;

/** ******************************************************************************************* **/
/**                                 SMTP(EMAIL) SETTINGS                                        **/
/** ******************************************************************************************* **/
#define SMTP_CONFIG_VERSION				(2) //(1->2) UTF-8 mutlilanguage
typedef struct
{
    BOOL                    smtp;                                   // control bit to ON/OFF email service
    CHAR                    server[MAX_SMTP_SERVER_NAME_WIDTH];     // specifies SMTP server name
    UINT16                  serverPort;                             // specifies SMTP port number
    CHAR                    username[MAX_SMTP_USERNAME_WIDTH];      // username for authentication
    CHAR                    password[MAX_SMTP_PASSWORD_WIDTH];      // password for authentication
    CHAR                    senderAddress[MAX_SMTP_EMAILID_WIDTH];  // sender's email address
    SMTP_ENCRYPTION_TYPE_e  encryptionType;                         // email encryption type

} SMTP_CONFIG_t;

/** ******************************************************************************************* **/
/**                                  FTP UPLOAD SETTINGS                                        **/
/** ******************************************************************************************* **/
#define FTP_UPLOAD_CONFIG_VERSION       (2) // (1->2) UTF-8 mutlilanguage
typedef struct
{
    BOOL 				ftp;                                    // control to enable/disable FTP service
    CHAR 				server[MAX_FTP_SERVER_NAME_WIDTH];      // specifies FTP server address
    UINT16 				serverPort;                             // specifies FTP server port, 21
    CHAR 				username[MAX_FTP_USERNAME_WIDTH];       // username for authentication at FTP server
    CHAR 				password[MAX_FTP_PASSWORD_WIDTH];       // password for authentication at FTP server
    CHAR 				uploadPath[MAX_FTP_UPLOAD_PATH_WIDTH];  // specifies the upload path for file

} FTP_UPLOAD_CONFIG_t;

/** ******************************************************************************************* **/
/**                                  TCP NOTIFY SETTINGS                                        **/
/** ******************************************************************************************* **/
#define TCP_NOTIFY_CONFIG_VERSION       (2) // (1->2) UTF-8 mutlilanguage
typedef struct
{
    BOOL 				tcpNotify;                              // control bit to ON/OFF TCP service, default is disable <0>
    CHAR 				server[MAX_TCP_SERVER_NAME_WIDTH];      // specifies TCP server Address (IP or host name)
    UINT16 				serverPort;                             // specifies TCP server Port address, default is 80

} TCP_NOTIFY_CONFIG_t;

/** ******************************************************************************************* **/
/**                                 FILE ACCESS SETTINGS                                        **/
/** ******************************************************************************************* **/
#define FILE_ACCESS_CONFIG_VERSION      (2) // (1->2) removed smb access
typedef struct
{
    BOOL 				ftpAccess;      // control enable/disable FTP access
    UINT16 				ftpport;        // specifies port number

} FILE_ACCESS_CONFIG_t;

/** ******************************************************************************************* **/
/**                                      HDD SETTINGS                                           **/
/** ******************************************************************************************* **/
#define HDD_CONFIG_VERSION              (1)
typedef struct
{
    HDD_MODE_e 			mode;           // one of the available HDD management mode
	RECORD_ON_DISK_e 	recordDisk;

} HDD_CONFIG_t;

/** ******************************************************************************************* **/
/**                              MATRIX DNS SERVER SETTINGS                                     **/
/** ******************************************************************************************* **/
#define MATRIX_DNS_SERVER_CONFIG_VERSION    (2) // (1->2) UTF-8 mutlilanguage
typedef struct
{
	BOOL 				enMacClient;
	CHAR 				hostName[MAX_MATRIX_DNS_SERVER_HOST_NAME_LEN];
	UINT16 				forwardedPort;

} MATRIX_DNS_SERVER_CONFIG_t;

/** ******************************************************************************************* **/
/**                                 USER ACCOUNT SETTINGS                                       **/
/** ******************************************************************************************* **/
#define USER_ACCOUNT_CONFIG_VERSION			(12) // 96 channel NVR support (camera config changed from 64 to 96)
typedef struct
{
    CHAR                            username[MAX_USER_ACCOUNT_USERNAME_WIDTH];                  // user name for account
    USER_GROUP_e                    userGroup;                                                  // group number which user belongs to
    CHAR                            password[MAX_USER_ACCOUNT_PASSWORD_WIDTH];                  // password for authentication
    BOOL                            userStatus;                                                 // User Enabled/ disabled
    BOOL                            multiLogin;                                                 // User allowed to have multiple-login
    UINT16                          loginSessionDuration;                                       // User Login Limit Duration
    USER_PRIVILEGE_u                userPrivilege[MAX_CAMERA_CONFIG];                           // camera privilege for user
    BOOL                            syncNetDev;
    CHAR                            preferredLanguage[MAX_LANGUAGE_FILE_NAME_LEN];
    BOOL                            managePushNotificationRights;

} USER_ACCOUNT_CONFIG_t;

/** ******************************************************************************************* **/
/**                                    CAMERA SETTINGS                                          **/
/** ******************************************************************************************* **/
#define CAMERA_CONFIG_VERSION					(6) // (5->6) Privacy mask are increased from 4 to 8 and text overlay from 1 to 6
typedef struct
{
    BOOL                    camera;                                                 // control to enable/disable camera
    CHAR                    name[MAX_CAMERA_NAME_WIDTH];                            // user defined name of camera
    CAMERA_TYPE_e           type;                                                   // camera type (Analog, IP or CI)
    BOOL                    logMotionEvents;                                        // whether to log motion events of the respective camera in the event log or not
    UINT8                   motionReDetectionDelay;                                 // (in seconds) On detection of Motion, maintain motion detection event till time
    OSD_POS_e               cameraNamePos;                                          // where to show camera name
    OSD_POS_e               statusPos;                                              // where to show status
    BOOL                    dateTimeOverlay;                                        // dateTimeOverlay is enable or not
    OSD_POS_e               dateTimePos;                                            // position for date-time
    BOOL                    channelNameOverlay;                                     // channelNameOverlay is enable or not
    CHAR                    channelName[TEXT_OVERLAY_MAX][MAX_CHANNEL_NAME_WIDTH];  // channel name
    OSD_POS_e               channelNamePos[TEXT_OVERLAY_MAX];                       // position for channel name
    CHAR                    mobileNum[MAX_MOBILE_NUM_WIDTH];                        // user defined name of camera
    BOOL                    privacyMaskStaus;                                       // enabled / disabled
    BOOL                    motionDetectionStatus;                                  // enabled / disabled
    VIDEO_TYPE_e            recordingStream;

} CAMERA_CONFIG_t;

/** ******************************************************************************************* **/
/**                                    STREAM SETTINGS                                          **/
/** ******************************************************************************************* **/
#define STREAM_CONFIG_VERSION			(3)
typedef struct
{
    CHAR 				videoEncoding[MAX_ENCODER_NAME_LEN];        // video encoding method
    CHAR 				resolution[MAX_RESOLUTION_NAME_LEN];        // image resolution
    UINT8 				framerate;                                  // frame rate
    UINT8 				quality;                                    // quality of stream
    BOOL 				enableAudio;                                // control to enable/disable audio
	BITRATE_MODE_e 		bitrateMode;
	BITRATE_VALUE_e 	bitrateValue;
	UINT8 				gop;
    CHAR 				videoEncodingSub[MAX_ENCODER_NAME_LEN];     // video encoding method
    CHAR 				resolutionSub[MAX_RESOLUTION_NAME_LEN];     // image resolution
    UINT8 				framerateSub;                               // frame rate
    UINT8 				qualitySub;                                 // quality of stream
    BOOL 				enableAudioSub;                             // control to enable/disable audio
	BITRATE_MODE_e 		bitrateModeSub;
	BITRATE_VALUE_e 	bitrateValueSub;
	UINT8 				gopSub;
	UINT16 				mainStreamProfile;
	UINT16 				subStreamProfile;

} STREAM_CONFIG_t;

/** ******************************************************************************************* **/
/**                             SCHEDULE RECORD SETTINGS                                        **/
/** ******************************************************************************************* **/
#define SCHEDULE_RECORD_CONFIG_VERSION  (2) // (1->2) Merged adaptive recording configurations
typedef struct
{
    BOOL     				scheduleRecording;                  // control to turn ON/OFF schedule recording
    DAY_RECORD_SCHEDULE_t   dailyRecord[MAX_WEEK_DAYS];         // weekly recording schedule for a camera

} SCHEDULE_RECORD_CONFIG_t;

/** ******************************************************************************************* **/
/**                               ALARM RECORD SETTINGS                                         **/
/** ******************************************************************************************* **/
#define ALARM_RECORD_CONFIG_VERSION		(1)
typedef struct
{
    UINT16  preRecordTime;              // duration in seconds, for which the stream should be buffered prior to alarm indication
    UINT16  postRecordTime;             // duration in seconds, for which the stream should be buffered after the alarm indication

} ALARM_RECORD_CONFIG_t;

/** ******************************************************************************************* **/
/**                          PTZ PRESET POSITIONS SETTINGS                                      **/
/** ******************************************************************************************* **/
#define PTZ_PRESET_CONFIG_VERSION       (3) // (2->3) ONVIF PTZ Preset token added
typedef struct
{
    CHAR    name[MAX_PRESET_POSITION_NAME_WIDTH];  // name of the respective PTZ preset
    CHAR    token[MAX_PRESET_TOKEN_LEN];           // token of the respective PTZ preset

} PTZ_PRESET_CONFIG_t;

/** ******************************************************************************************* **/
/**                               PRESET TOUR SETTINGS                                          **/
/** ******************************************************************************************* **/
#define PRESET_TOUR_CONFIG_VERSION      (4) // (3->4) UTF-8 mutlilanguage
typedef struct
{
    BOOL    manualTourOverride;         // control bit to turn ON/OFF Manual override over schedule tour
    UINT8   manualTour;                 // number of tour to be initiated manually
    TOUR_t  tour[MAX_TOUR_NUMBER];      // tours settings
    BOOL    activeTourOverride;         // overide active tour by Ptz Control

} PRESET_TOUR_CONFIG_t;

/** ******************************************************************************************* **/
/**                           PRESET TOUR SCHEDULE SETTINGS                                     **/
/** ******************************************************************************************* **/
#define TOUR_SCHEDULE_CONFIG_VERSION    (1)
typedef struct
{
	DAILY_TOUR_SCHEDULE_t tour[MAX_WEEK_DAYS];

} TOUR_SCHEDULE_CONFIG_t;

/** ******************************************************************************************* **/
/**                            SYSTEM SENSOR INPUT SETTINGS                                     **/
/** ******************************************************************************************* **/
#define SENSOR_CONFIG_VERSION           (2) // (1->2) UTF-8 mutlilanguage
typedef struct
{
    BOOL 				sensorDetect;                   // control bit to turn ON/OFF detection of the sensor
    CHAR 				name[MAX_SENSOR_NAME_WIDTH];    // user defined name of the sensor
    SENSOR_MODE_e 		normalMode;                     // normal mode option [NO- normally open / NC- normally close]
    UINT8 				debounceTime;                   // debounce time in seconds

} SENSOR_CONFIG_t;

/** ******************************************************************************************* **/
/**                            SYSTEM ALARM OUTPUT SETTINGS                                     **/
/** ******************************************************************************************* **/
#define ALARM_CONFIG_VERSION            (2) // (1->2) UTF-8 mutlilanguage
typedef struct
{
    BOOL 				alarmOutput;                    // control bit to turn ON/OFF alarm detection
    CHAR 				name[MAX_ALARM_NAME_WIDTH];     // user defined alarm name
    ALARM_MODE_e 		activeMode;                     // alarm activation mode, event based or time period based
    UINT8 				pulsePeriod;                    // time period in seconds for which alarm should be keep active

} ALARM_CONFIG_t;

/** ******************************************************************************************* **/
/**                               UPLOAD IMAGE SETTINGS                                         **/
/** ******************************************************************************************* **/
#define IMAGE_UPLOAD_CONFIG_VERSION		(4) // (2->3) UTF-8 mutlilanguage
typedef struct
{
    CHAR 				resolution[MAX_RESOLUTION_NAME_LEN];    // upload image resolution
    UPLOAD_LOCATION_e 	uploadLocation;                         // image upload location
    EMAIL_PARAMETER_t 	uploadToEmail;                          // email parameter to upload the image
    UINT8 				uploadImageRate;                        // upload rate in minutes

} IMAGE_UPLOAD_CONFIG_t;

/** ******************************************************************************************* **/
/**                                   STORAGE SETTINGS                                          **/
/** ******************************************************************************************* **/
#define STORAGE_CONFIG_VERSION		(3)                         // 96 channel NVR support (camera config changed from 64 to 96)
typedef struct
{
    HDD_FULL_t              hddFull;                            // parameters for HDD full event
    BOOL                    recordRetentionStatus;
	RECORD_RETENTION_TYPE_e recordRetentionType;
    UINT8                   driveWiseRecordCleanupByDays;       // number of days, in case of "older then days" cleanup option
    STORAGE_ALERT_t         hddStorage;                         // parameters for storage alert
    BOOL                    backUpRetentionStatus;
    UINT16                  cameraWiseRecordCleanupByDays[MAX_CAMERA_CONFIG];
    UINT16                  cameraWiseBackUpCleanupByDays[MAX_CAMERA_CONFIG];

} STORAGE_CONFIG_t;

/** ******************************************************************************************* **/
/**                            SCHEDULE BACKUP SETTINGS                                         **/
/** ******************************************************************************************* **/
#define SCHEDULE_BACKUP_CONFIG_VERSION		(4) // 96 channel NVR support (camera config changed from 64 to 96)
typedef struct
{
    BOOL                    scheduleBackup;     // Flag to indicate statsu of schedule backup
    SB_MODE_e               mode;               // schedule backup mode, hourly, daily, weekly
    SB_EVERY_HOUR_MODE_e    everyHourMode;      // schedule back every hour mode
    SB_EVERYDAY_CONFIG_t    everydayBackup;     // daily backup schedule
    SB_WEEKLY_CONFIG_t      weeklyBackup;       // weekly backup schedule
    CAMERA_BIT_MASK_t       camera;             // Bit-Wise flags to indicate if schedule back-up is enabled /disabled for particular camera
    SB_LOCATION_e           backupLocation;     // backup location

} SCHEDULE_BACKUP_CONFIG_t;

/** ******************************************************************************************* **/
/**                               MANUAL BACKUP SETTINGS                                        **/
/** ******************************************************************************************* **/
#define MANUAL_BACKUP_CONFIG_VERSION		(5) // 96 channel NVR support (camera config changed from 64 to 96)
typedef struct
{
    BOOL 				manualBackup;           // Flag to indicate status of manual backup
    MB_DURATION_e 		duration;               // backup duration
    CAMERA_BIT_MASK_t   camera;                 // Bit-Wise flags to indicate if
    MB_LOCATION_e 		backupLocation;         // backup location

} MANUAL_BACKUP_CONFIG_t;

/** ******************************************************************************************* **/
/**                               CAMERA ALARM SETTINGS                                         **/
/** ******************************************************************************************* **/
#define CAMERA_ALARM_CONFIG_VERSION		(1)
typedef struct
{
    ALARM_MODE_e 		activeMode;     // alarm mode
    UINT8 				activeTime;     // duration for which alarm should be kept on

} CAMERA_ALARM_CONFIG_t;

/** ******************************************************************************************* **/
/**                       CAMERA EVENT AND ACTION SETTINGS                                      **/
/** ******************************************************************************************* **/
#define CAMERA_EVENT_CONFIG_VERSION		(9) // (8->9) 96 channel NVR support (camera config changed from 64 to 96)
typedef struct
{
    BOOL                        action;                         // control bit to ON/OFF action against an event
    ACTION_PARAMETERS_t         actionParam;
    WEEKLY_ACTION_SCHEDULE_t    weeklySchedule[MAX_WEEK_DAYS];  // weekly action schedule <time and action list>
    CAMERA_BIT_MASK_t           copyToCam;

} CAMERA_EVENT_CONFIG_t;

/** ******************************************************************************************* **/
/**                       SENSOR EVENT AND ACTION SETTINGS                                      **/
/** ******************************************************************************************* **/
#define SENSOR_EVENT_CONFIG_VERSION		(5) // (4->5) 96 channel NVR support (camera config changed from 64 to 96)
typedef struct
{
    BOOL                        action;                         // control bit to ON/OFF action against an event
    ACTION_PARAMETERS_t         actionParam;
    WEEKLY_ACTION_SCHEDULE_t    weeklySchedule[MAX_WEEK_DAYS];  // weekly action schedule <time and action list>

} SENSOR_EVENT_CONFIG_t;

/** ******************************************************************************************* **/
/**                       SYSTEM EVENT AND ACTION SETTINGS                                      **/
/** ******************************************************************************************* **/
#define SYSTEM_EVENT_CONFIG_VERSION		(7) // (6->7) 96 channel NVR support (camera config changed from 64 to 96)
typedef struct
{
    BOOL                        action;                         // control bit to ON/OFF action against an event
    ACTION_PARAMETERS_t         actionParam;
    ACTION_BIT_u                actionBits;                     // permanent action control bits

} SYSTEM_EVENT_CONFIG_t;

/** ******************************************************************************************* **/
/**                            COSEC PRE-RECORD SETTINGS                                        **/
/** ******************************************************************************************* **/
#define COSEC_PREALRM_CONFIG_VERSION	(1)
typedef struct
{
	BOOL 				enable;
	UINT8 				preRecDuration;
} COSEC_REC_PARAM_CONFIG_t;

/** ******************************************************************************************* **/
/**                              STATIC ROUTING SETTINGS                                        **/
/** ******************************************************************************************* **/
#define STATIC_ROUTING_CONFIG_VERSION	(4) // (3->4) Ipv6 support added
typedef struct
{
    NETWORK_PORT_e      defaultPort;                            // default Routing Port
    ROUTING_ENTRY_t     entry[MAX_STATIC_ROUTING_ENTRY];
} STATIC_ROUTING_CONFIG_t;

/** ******************************************************************************************* **/
/**                            MOBILE BROADBAND SETTINGS                                        **/
/** ******************************************************************************************* **/
#define BROAD_BAND_CONFIG_VERSION       (4) // (3->4) Service type removed
typedef struct
{
	UINT8 					activeProfile;
	BROAD_BAND_PROFILE_t 	broadBandCfg[MAX_BROADBAND_PROFILE];
} BROAD_BAND_CONFIG_t;

/** ******************************************************************************************* **/
/**                                     SMS SETTINGS                                            **/
/** ******************************************************************************************* **/
#define SMS_CONFIG_VERSION				(3) // (2->3) smslane v2 support added
typedef struct
{
	SMS_MODE_e 			mode;
	SMS_SERVER_e 		smsServer;
	CHAR			    userName[MAX_SMS_ACC_USERNAME_WIDTH];
	CHAR 				password[MAX_SMS_ACC_PASSWORD_WIDTH];
	CHAR 				sendorId[MAX_SMS_ACC_SENDOR_ID_WIDTH];
	BOOL 				sendAsFlash;

} SMS_CONFIG_t;

/** ******************************************************************************************* **/
/**                              MANUAL RECORD SETTINGS                                         **/
/** ******************************************************************************************* **/
#define MANUAL_RECORD_CONFIG_VERSION    (1)
typedef struct
{
    BOOL 				manualRecordStatus; // Enabled / disabled for respective camera
} MANUAL_RECORD_CONFIG_t;

/** ******************************************************************************************* **/
/**                              NETWORK DRIVE SETTINGS                                         **/
/** ******************************************************************************************* **/
#define NETWORK_DRIVE_CONFIG_VERSION	(4) // (3->4) Added IPv6 Support
typedef struct
{
	BOOL 				enable;
	CHAR 				name[MAX_NW_DRIVE_NAME_LEN];
    CHAR 				ipAddr[IPV6_ADDR_LEN_MAX];
	CHAR 				userName[MAX_NW_DRIVE_USER_NAME_LEN];
	CHAR 				password[MAX_NW_DRIVE_PASSWORD_LEN];
	FILE_SYS_TYPE_e 	fileSys;
	CHAR 				dfltFolder[MAX_NW_DRIVE_DFLT_FOLDER_PATH_LEN];
} NETWORK_DRIVE_CONFIG_t;

/** ******************************************************************************************* **/
/**                                 IP CAMERA SETTINGS                                          **/
/** ******************************************************************************************* **/
#define IP_CAMERA_CONFIG_VERSION        (10) //(9->10) /* Axis camera support removal */
typedef struct
{
    CHAR            brand[MAX_BRAND_NAME_LEN];                  // camera brand name
    CHAR            model[MAX_MODEL_NAME_LEN];                  // camera model name
    CHAR            url[MAX_CAMERA_URL_WIDTH];                  // url to be used for generic camera
    CHAR            cameraAddress[MAX_CAMERA_ADDRESS_WIDTH];    // ip address of camera
    UINT16          httpPort;                                   // http port of camera
    UINT16          rtspPort;                                   // rtsp port number of camera
    CHAR            username[MAX_CAMERA_USERNAME_WIDTH];        // user name for authentication of camera
    CHAR            password[MAX_CAMERA_PASSWORD_WIDTH];        // password for authentication of camera
    BOOL            onvifSupportF;                              // Camera is ONVIF supported
    UINT16          onvifPort;                                  // ONVIF port
    RTSP_PROTOCOL_e rtspProtocol;                               // RTSP PROTOCOL
    CHAR            macAddr[MAX_MAC_ADDRESS_WIDTH];

} IP_CAMERA_CONFIG_t;

/** ******************************************************************************************* **/
/**                                   NETWORK DEVICE SETTINGS                                   **/
/** ******************************************************************************************* **/
#define NETWORK_DEVICE_CONFIG_VERSION   (5) // (4->5) UTF-8 mutlilanguage
typedef struct
{
    CHAR 				deviceName[MAX_NETWORK_DEVICE_NAME_LEN];            // network device name
    REGISTER_MODE_e 	registerMode;                                       // network register mode for connect to the network device
	CHAR 				registerModeAddress[MAX_REGISTER_MODE_ADDRESS_LEN]; // register Mode address of network device
    UINT16 				port;                                               // port address of network device
    CHAR 				username[MAX_NETWORK_DEVICE_USERNAME_LEN];          // username of network device
    CHAR 				password[MAX_NETWORK_DEVICE_PASSWORD_LEN];          // password of network device
    BOOL 				enable;                                             // network device enable / disable
    BOOL 				autoLogin;                                          // auto login to network device enable / disable
    LIVE_STREAM_TYPE_e  liveStreamType;                                     // live stream type
    BOOL 				preferDeviceCredential;                             // For Prefering Native Device Credentials. Enable/Disable
	UINT16				forwardedTcpPort;
} NETWORK_DEVICE_CONFIG_t;

/** ******************************************************************************************* **/
/**                                     SNAPSHOT SETTINGS                                       **/
/** ******************************************************************************************* **/
#define SNAPSHOT_CONFIG_VERSION			(3) // (2->3) max image upload rate limit changed to 5
typedef struct
{
    BOOL 				snapShotEnable;             // snapshot Enable
    UPLOAD_LOCATION_e 	snapShotuploadLocation;     // image upload location
    EMAIL_PARAMETER_t 	snapShotuploadToEmail;      // email parameter to upload the image
    UINT8 				snapShotuploadImageRate;    // upload rate in minutes

} SNAPSHOT_CONFIG_t;

/** ******************************************************************************************* **/
/**                                SNAPSHOT SCHEDULE SETTINGS                                   **/
/** ******************************************************************************************* **/
#define SNAPSHOT_SCHEDULE_CONFIG_VERSION    (1)
typedef struct
{
    DAY_RECORD_SCHEDULE_t dailySanpShotPeriod;      // weekly snapShot schedule for a camera
	UINT8 				  copyToWeekDays;

} SNAPSHOT_SCHEDULE_CONFIG_t;

/** ******************************************************************************************* **/
/**                                    LOGIN POLICY SETTINGS                                    **/
/** ******************************************************************************************* **/
#define LOGIN_POLICY_CONFIG_VERSION			(2)
typedef struct
{
	UINT8 					minPassLength;
	BOOL 					passResetStatus;
	UINT16 					passResetPeriod;
	LOGIN_LOCK_STATUS_e 	lockAccountStatus;
	UINT16 					invalidLoginAttempt;
	UINT16 					autoLockTimer;

} LOGIN_POLICY_CONFIG_t;

/** ******************************************************************************************* **/
/**                                      AUDIO-OUT SETTINGS                                     **/
/** ******************************************************************************************* **/
#define AUDIO_OUT_CONFIG_VERSION            (1)
typedef struct
{
    UINT8   priority;

} AUDIO_OUT_CONFIG_t;

/** ******************************************************************************************* **/
/**                                         P2P SETTINGS                                        **/
/** ******************************************************************************************* **/
#define P2P_CONFIG_VERSION                  (2) // (1 --> 2) Fallback to relay server flag added
typedef struct
{
    BOOL    p2pEnable;
    BOOL    fallbackToRelayServer;
	
} P2P_CONFIG_t;

/** ******************************************************************************************* **/
/**                                       IMAGE SETTINGS                                        **/
/** ******************************************************************************************* **/
#define IMAGE_SETTING_CONFIG_VERSION        (1)
typedef struct
{
    UINT8   brightness;
    UINT8   contrast;
    UINT8   saturation;
    UINT8   hue;
    UINT8   sharpness;
    UINT8   whiteBalanceMode;
    UINT8   wdrMode;
    UINT8   wdrStrength;
    UINT8   backlightMode;
    UINT8   exposureRatioMode;
    UINT8   exposureRatio;
    UINT8   exposureMode;
    UINT8   flicker;
    UINT8   flickerStrength;
    UINT8   hlc;
    UINT32  exposureTime;
    UINT8   exposureGain;
    UINT8   exposureIris;
    UINT8   normalLightGain;
    UINT8   normalLightLuminance;
    UINT8   irLedMode;
    UINT8   irLedSensitivity;

}IMG_SETTING_CONFIG_t;

/** ******************************************************************************************* **/
/**                                         DHCP SERVER                                         **/
/** ******************************************************************************************* **/
#define DHCP_SERVER_CONFIG_VERSION          (1)
typedef struct
{
    BOOL            enable;
    LAN_CONFIG_ID_e lanPort;
    CHAR            startIpAddr[IPV4_ADDR_LEN_MAX];
    UINT16          noOfHost;
    CHAR            gatewayAddr[IPV4_ADDR_LEN_MAX];
    CHAR            dnsServerAddr[IPV4_ADDR_LEN_MAX];
    UINT16          leaseHour;

}DHCP_SERVER_CONFIG_t;

/** ******************************************************************************************* **/
/**                                    FIRMWARE MANAGEMENT                                      **/
/** ******************************************************************************************* **/
#define FIRMWARE_MANAGEMENT_CONFIG_VERSION  (2) // (1->2) Matrix's FTP server path updated
typedef struct
{
    AUTO_FIRMWARE_UPGRADE_MODE_e    autoUpgradeMode;
    TIME_HH_MM_t                    scheduleTime;      /* Daily schedule check time */
    FIRMWARE_FTP_SERVER_TYPE_e      ftpServerType;
    FTP_UPLOAD_CONFIG_t             ftpServerInfo[FIRMWARE_FTP_SERVER_MAX];

}FIRMWARE_MANAGEMENT_CONFIG_t;

/** ******************************************************************************************* **/
/**                                     FCM PUSH NOTIFICATION                                   **/
/** ******************************************************************************************* **/
#define FCM_PUSH_NOTIFY_CONFIG_VERSION	(1)
typedef struct
{
    CHAR                    username[MAX_USER_ACCOUNT_USERNAME_WIDTH];
    CHAR                    deviceFcmToken[FCM_TOKEN_LENGTH_MAX];
    CHAR                    deviceModelName[FCM_DEVICE_MODEL_NAME_LEN_MAX];

} FCM_PUSH_NOTIFY_CONFIG_t;

/** ******************************************************************************************* **/
/**                                       PASSWORD RECOVERY                                     **/
/** ******************************************************************************************* **/
#define PASSWORD_RECOVERY_CONFIG_VERSION    (1)
typedef struct
{
    CHAR                            emailId[PWD_RECOVERY_EMAIL_ID_LEN_MAX];                     // user email id for reset password functionality
    USER_SECURITY_QUESTION_INFO_t   securityQuestionInfo[PWD_RECOVERY_SECURITY_QUESTION_MAX];   // security questions info(id & ans) for pswd reset functionality

} PASSWORD_RECOVERY_CONFIG_t;

/** ******************************************************************************************* **/
/**                                      STORAGE ALLOCATION                                     **/
/** ******************************************************************************************* **/
#define STORAGE_ALLOCATION_CONFIG_VERSION   (1)
typedef struct
{
    STORAGE_ALLOCATION_INFO_t       storageAllocation[STORAGE_ALLOCATION_GROUP_MAX];    /* Storage allocation to storage group */

} STORAGE_ALLOCATION_CONFIG_t;

//#################################################################################################
// @EXTERN VARIABLE
//#################################################################################################
extern  	 GENERAL_CONFIG_t               DfltGeneralCfg;
extern const DATE_TIME_CONFIG_t             DfltDateTimeCfg;
extern const DST_CONFIG_t                   DfltDstCfg;
extern const LAN_CONFIG_t                   DfltLanCfg[MAX_LAN_PORT];
extern const MAC_SERVER_CNFG_t              DfltMacServerCfg;
extern const IP_FILTER_CONFIG_t             DfltIpFilterCfg;
extern const DDNS_CONFIG_t                  DfltDdnsCfg;
extern const SMTP_CONFIG_t                  DfltSmtpCfg;
extern const FTP_UPLOAD_CONFIG_t            DfltFtpUploadCfg;
extern const TCP_NOTIFY_CONFIG_t            DfltTcpNotifyCfg;
extern const FILE_ACCESS_CONFIG_t           DfltFileAccessCfg;
extern const HDD_CONFIG_t                   DfltHddCfg;
extern const MATRIX_DNS_SERVER_CONFIG_t     DfltMatrixDnsServerCfg;
extern const USER_ACCOUNT_CONFIG_t          DfltUserAccountCfg;
extern const CAMERA_CONFIG_t                DfltCameraCfg;
extern const STREAM_CONFIG_t                DfltCameraStreamCfg;
extern const SCHEDULE_RECORD_CONFIG_t       DfltScheduleRecordCfg;
extern const ALARM_RECORD_CONFIG_t          DfltAlarmRecordCfg;
extern const PTZ_PRESET_CONFIG_t            DfltPtzPresetCfg;
extern const PRESET_TOUR_CONFIG_t           DfltPresetTourCfg;
extern const TOUR_SCHEDULE_CONFIG_t         DfltTourScheduleCfg;
extern const IMAGE_UPLOAD_CONFIG_t          DfltImageUploadCfg;
extern const SENSOR_CONFIG_t                DfltSensorCfg;
extern const ALARM_CONFIG_t                 DfltAlarmCfg;
extern const STORAGE_CONFIG_t               DfltStorageCfg;
extern const SCHEDULE_BACKUP_CONFIG_t       DfltScheduleBackupCfg;
extern const MANUAL_BACKUP_CONFIG_t         DfltManualBackupCfg;
extern const CAMERA_ALARM_CONFIG_t          DfltCameraAlarmCfg;
extern const CAMERA_EVENT_CONFIG_t          DfltCameraEventActionCfg;
extern const SENSOR_EVENT_CONFIG_t          DfltSensorEventActionCfg;
extern const SYSTEM_EVENT_CONFIG_t          DfltSystemEventActionCfg;
extern const COSEC_REC_PARAM_CONFIG_t       DfltCosecPreRecConfig;
extern const STATIC_ROUTING_CONFIG_t        DfltStaticRoutingCfg;
extern const BROAD_BAND_CONFIG_t            DfltBroadBandCfg;
extern const SMS_CONFIG_t                   DfltSmsCfg;
extern const MANUAL_RECORD_CONFIG_t         DfltManualRecordCfg;
extern const NETWORK_DRIVE_CONFIG_t         DfltNetworkDriveCfg;
extern const IP_CAMERA_CONFIG_t             DfltIpCameraCfg;
extern const NETWORK_DEVICE_CONFIG_t        DfltNetworkDeviceCfg;
extern const SNAPSHOT_CONFIG_t              DfltSnapShotCfg;
extern const SNAPSHOT_SCHEDULE_CONFIG_t     DfltSnapShotSchdCfg;
extern const LOGIN_POLICY_CONFIG_t          DfltLoginPolicyCfg;
extern const AUDIO_OUT_CONFIG_t             DfltAudioOutCfg;
extern const P2P_CONFIG_t                   DfltP2PCfg;
extern const IMG_SETTING_CONFIG_t           DfltImageSettingCfg;
extern const DHCP_SERVER_CONFIG_t           DfltDhcpServerCfg;
extern const FIRMWARE_MANAGEMENT_CONFIG_t   DfltFirmwareManagementCfg;
extern const FCM_PUSH_NOTIFY_CONFIG_t       DfltFcmPushNotificationCfg;
extern const PASSWORD_RECOVERY_CONFIG_t     DfltPasswordRecoveryCfg;
extern const STORAGE_ALLOCATION_CONFIG_t    DfltStorageAllocationCfg;

/* Camera config default status variable */
extern BOOL cameraConfigDefaultF[MAX_CAMERA];

/* Current configuration copy */
extern GENERAL_CONFIG_t                     generalCfg;
extern DATE_TIME_CONFIG_t                   dateTimeCfg;
extern DST_CONFIG_t                         dstCfg;
extern LAN_CONFIG_t                         lanCfg[MAX_LAN_PORT];
extern MAC_SERVER_CNFG_t                    macServerCfg;
extern IP_FILTER_CONFIG_t                   ipFilterCfg;
extern DDNS_CONFIG_t                        ddnsCfg;
extern SMTP_CONFIG_t                        smtpCfg;
extern FTP_UPLOAD_CONFIG_t                  ftpUploadCfg[MAX_FTP_SERVER];
extern TCP_NOTIFY_CONFIG_t                  tcpNotifyCfg;
extern FILE_ACCESS_CONFIG_t                 fileAccessCfg;
extern HDD_CONFIG_t                         hddCfg;
extern MATRIX_DNS_SERVER_CONFIG_t           matrixDnsServerCfg;
extern USER_ACCOUNT_CONFIG_t                userAccountCfg[MAX_USER_ACCOUNT];
extern CAMERA_CONFIG_t                      cameraCfg[MAX_CAMERA];
extern STREAM_CONFIG_t                      streamCfg[MAX_CAMERA];
extern SCHEDULE_RECORD_CONFIG_t             scheduleRecordCfg[MAX_CAMERA];
extern ALARM_RECORD_CONFIG_t                alarmRecordCfg[MAX_CAMERA];
extern PTZ_PRESET_CONFIG_t                  ptzPresetCfg[MAX_CAMERA][MAX_PRESET_POSITION];
extern PRESET_TOUR_CONFIG_t                 presetTourCfg[MAX_CAMERA];
extern TOUR_SCHEDULE_CONFIG_t               tourScheduleCfg[MAX_CAMERA];
extern IMAGE_UPLOAD_CONFIG_t                imageUploadCfg[MAX_CAMERA];
extern SENSOR_CONFIG_t                      sensorCfg[MAX_SENSOR];
extern ALARM_CONFIG_t                       alarmCfg[MAX_ALARM];
extern STORAGE_CONFIG_t                     storageCfg;
extern SCHEDULE_BACKUP_CONFIG_t             scheduleBackupCfg;
extern MANUAL_BACKUP_CONFIG_t               manualBackupCfg;
extern CAMERA_ALARM_CONFIG_t                cameraAlarmCfg[MAX_CAMERA][MAX_CAMERA_ALARM];
extern CAMERA_EVENT_CONFIG_t                cameraEventCfg[MAX_CAMERA][MAX_CAMERA_EVENT];
extern SENSOR_EVENT_CONFIG_t                sensorEventCfg[MAX_SENSOR_EVENT];
extern SYSTEM_EVENT_CONFIG_t                systemEventCfg[MAX_SYSTEM_EVENT];
extern COSEC_REC_PARAM_CONFIG_t             cosecPreRecCfg[MAX_CAMERA];
extern STATIC_ROUTING_CONFIG_t              staticRoutingCfg;
extern BROAD_BAND_CONFIG_t                  broadBandCfg;
extern SMS_CONFIG_t                         smsCfg;
extern MANUAL_RECORD_CONFIG_t               manualRecordCfg[MAX_CAMERA];
extern NETWORK_DRIVE_CONFIG_t               nwDriveCfg[MAX_NW_DRIVE];
extern IP_CAMERA_CONFIG_t                   ipCameraCfg[MAX_CAMERA];
extern NETWORK_DEVICE_CONFIG_t              networkDeviceCfg[MAX_NETWORK_DEVICES];
extern SNAPSHOT_CONFIG_t                    snapShotCfg[MAX_CAMERA];
extern SNAPSHOT_SCHEDULE_CONFIG_t           snapShotSchdCfg[MAX_CAMERA];
extern LOGIN_POLICY_CONFIG_t                loginPolicyCfg;
extern AUDIO_OUT_CONFIG_t                   audioOutCfg;
extern P2P_CONFIG_t                         p2pCfg;
extern IMG_SETTING_CONFIG_t                 imageSettingCfg[MAX_CAMERA];
extern DHCP_SERVER_CONFIG_t                 dhcpServerCfg;
extern FIRMWARE_MANAGEMENT_CONFIG_t         firmwareManagementCfg;
extern FCM_PUSH_NOTIFY_CONFIG_t             fcmPushNotificationCfg[FCM_PUSH_NOTIFY_DEVICES_MAX];
extern PASSWORD_RECOVERY_CONFIG_t           passwordRecoveryCfg[MAX_USER_ACCOUNT];
extern STORAGE_ALLOCATION_CONFIG_t          storageAllocationCfg;

//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* CONFIGURATIONMODULE_H_ */
