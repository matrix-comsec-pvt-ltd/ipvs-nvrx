#if !defined CONFIGURATIONOLD_H
#define CONFIGURATIONOLD_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   ConfigOld.h
@brief  This file contains older version configuration data structures [structure, macro and enum]
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "ConfigComnDef.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
/** ********* GENERAL SETTINGS ********** **/
// Change File version for Default Value changed for Quality parameter
typedef struct
{
    CHAR 				deviceName[MAX_DEVICE_NAME_WIDTH_V1];       // Device Name of the NVR module
    UINT8 				fileRecordDuration;                         // single file Record duration, in minutes
    UINT8 				autoLogoutDuration;                         // provides duration of auto logout feature, in minutes
    BOOL 				autoPowerOn;                                // Flag indicating whether to resume working after power is up
    UINT16 				httpPort;                                   // HTTP Server listen port
    UINT16 				tcpPort;                                    // TCP Server listen port (for control communication with PC client)
    RTP_PORT_RANGE_t    rtpPort;                                    // RTP port range used for media streams
    UINT16			    onvifPort;                                  // ONVIF port
    UINT8 				deviceId;                                   // Device id
    CAM_VIDEO_SYSTEM_e  videoSystemType;                            // NTSC / PAL
    BOOL 				integrateCosec;                             // Integrate with cosec
    DATE_FORMAT_e 		dateFormat;                                 // format for date
    TIME_FORMAT_e 		timeFormat;                                 // format for time
    RECORD_FORMAT_TYPE_e recordFormatType;                          // format for time
    UINT8 				analogCamNo;                                // Analog Cam Number
    UINT8 				ipCamNo;                                    // IP Cam Number
    // Auto configure camera related parameter
    BOOL 				autoConfigureCameraFlag;                    // camrea auto configure flag
    BOOL 				retainIpAddresses;                          // Ip address retain flag
    CHAR			    autoConfigStartIp[IPV4_ADDR_LEN_MAX];       // auto config start ip
    CHAR 				autoConfigEndIp[IPV4_ADDR_LEN_MAX];         // auto config end ips
    BOOL 				retainDfltProfile;                          // retain profile default parameter
    CHAR 				videoEncoding[MAX_ENCODER_NAME_LEN];        // video encoding method
    CHAR 				resolution[MAX_RESOLUTION_NAME_LEN];        // image resolution
    UINT8 				framerate;                                  // frame rate
    UINT8 				quality;                                    // quality of stream
    BOOL 				enableAudio;                                // control to enable/disable audio
    BITRATE_MODE_e		bitrateMode;
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
    BOOL 				devInitWithServerF;                         // intigrate with samas Flag
    CHAR 				devInitServerIpAddr[IPV4_ADDR_LEN_MAX];     // samas Server Ip addr
    UINT16 				devInitServerPort;                          // samas Server Port
    BOOL 				autoRecordFailFlag;
    UINT16				videoPopupDuration;
    UINT16				preVideoLossDuration;
    CHAR 				userName[MAX_USERNAME_WIDTH_V1];
    CHAR 				password[MAX_PASSWORD_WIDTH_V1];
    BOOL                startLiveView;                              // For Decoding in Local Client
    UINT16				forwardedTcpPort;

} GENERAL_CONFIG_VER12_t;

typedef struct
{
    CHAR 				deviceName[MAX_DEVICE_NAME_WIDTH_V1];       // Device Name of the NVR module
    UINT8 				fileRecordDuration;                         // single file Record duration, in minutes
    UINT8 				autoLogoutDuration;                         // provides duration of auto logout feature, in minutes
    BOOL 				autoPowerOn;                                // Flag indicating whether to resume working after power is up
    UINT16 				httpPort;                                   // HTTP Server listen port
    UINT16 				tcpPort;                                    // TCP Server listen port (for control communication with PC client)
    RTP_PORT_RANGE_t    rtpPort;                                    // RTP port range used for media streams
    UINT16			    onvifPort;                                  // ONVIF port
    UINT8 				deviceId;                                   // Device id
    CAM_VIDEO_SYSTEM_e  videoSystemType;                            // NTSC / PAL
    BOOL 				integrateCosec;                             // Integrate with cosec
    DATE_FORMAT_e 		dateFormat;                                 // format for date
    TIME_FORMAT_e 		timeFormat;                                 // format for time
    RECORD_FORMAT_TYPE_e recordFormatType;                          // format for time
    UINT8 				analogCamNo;                                // Analog Cam Number
    UINT8 				ipCamNo;                                    // IP Cam Number
	// Auto configure camera related parameter
    BOOL 				autoConfigureCameraFlag;                    // camrea auto configure flag
    BOOL 				retainIpAddresses;                          // Ip address retain flag
    CHAR			    autoConfigStartIp[IPV4_ADDR_LEN_MAX];       // auto config start ip
    CHAR 				autoConfigEndIp[IPV4_ADDR_LEN_MAX];         // auto config end ips
    BOOL 				retainDfltProfile;                          // retain profile default parameter
    CHAR 				videoEncoding[MAX_ENCODER_NAME_LEN];        // video encoding method
    CHAR 				resolution[MAX_RESOLUTION_NAME_LEN];        // image resolution
    UINT8 				framerate;                                  // frame rate
    UINT8 				quality;                                    // quality of stream
    BOOL 				enableAudio;                                // control to enable/disable audio
	BITRATE_MODE_e		bitrateMode;
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
    BOOL 				devInitWithServerF;                         // intigrate with samas Flag
    CHAR 				devInitServerIpAddr[IPV4_ADDR_LEN_MAX];     // samas Server Ip addr
    UINT16 				devInitServerPort;                          // samas Server Port
	BOOL 				autoRecordFailFlag;
	UINT16				videoPopupDuration;
	UINT16				preVideoLossDuration;
	CHAR 				userName[MAX_USERNAME_WIDTH_V1];
	CHAR 				password[MAX_PASSWORD_WIDTH_V1];
    BOOL                startLiveView;                              // For Decoding in Local Client
	UINT16				forwardedTcpPort;
	BOOL				autoAddCamFlag;
	UINT16				autoAddCamTcpPort;
	UINT8				pollDuration;
	UINT8				pollInterval;

} GENERAL_CONFIG_VER13_t;

typedef struct
{
    CHAR 				deviceName[MAX_DEVICE_NAME_WIDTH_V1];       // Device Name of the NVR module
    UINT8 				fileRecordDuration;                         // single file Record duration, in minutes
    UINT8 				autoLogoutDuration;                         // provides duration of auto logout feature, in minutes
    BOOL 				autoPowerOn;                                // Flag indicating whether to resume working after power is up
    UINT16 				httpPort;                                   // HTTP Server listen port
    UINT16 				tcpPort;                                    // TCP Server listen port (for control communication with PC client)
    RTP_PORT_RANGE_t    rtpPort;                                    // RTP port range used for media streams
    UINT16			    onvifPort;                                  // ONVIF port
    UINT8 				deviceId;                                   // Device id
    CAM_VIDEO_SYSTEM_e  videoSystemType;                            // NTSC / PAL
    BOOL 				integrateCosec;                             // Integrate with cosec
    DATE_FORMAT_e 		dateFormat;                                 // format for date
    TIME_FORMAT_e 		timeFormat;                                 // format for time
    RECORD_FORMAT_TYPE_e recordFormatType;                          // format for time
    UINT8 				analogCamNo;                                // Analog Cam Number
    UINT8 				ipCamNo;                                    // IP Cam Number
	// Auto configure camera related parameter
    BOOL 				autoConfigureCameraFlag;                    // camrea auto configure flag
    BOOL 				retainIpAddresses;                          // Ip address retain flag
    CHAR			    autoConfigStartIp[IPV4_ADDR_LEN_MAX];       // auto config start ip
    CHAR 				autoConfigEndIp[IPV4_ADDR_LEN_MAX];         // auto config end ips
    BOOL 				retainDfltProfile;                          // retain profile default parameter
    CHAR 				videoEncoding[MAX_ENCODER_NAME_LEN];        // video encoding method
    CHAR 				resolution[MAX_RESOLUTION_NAME_LEN];        // image resolution
    UINT8 				framerate;                                  // frame rate
    UINT8 				quality;                                    // quality of stream
    BOOL 				enableAudio;                                // control to enable/disable audio
	BITRATE_MODE_e		bitrateMode;
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
    BOOL 				devInitWithServerF;                         // intigrate with samas Flag
    CHAR 				devInitServerIpAddr[IPV4_ADDR_LEN_MAX];     // samas Server Ip addr
    UINT16 				devInitServerPort;                          // samas Server Port
	BOOL 				autoRecordFailFlag;
	UINT16				videoPopupDuration;
	UINT16				preVideoLossDuration;
	CHAR 				userName[MAX_USERNAME_WIDTH_V1];
	CHAR 				password[MAX_PASSWORD_WIDTH_V1];
    BOOL                startLiveView;                              // For Decoding in Local Client
	UINT16				forwardedTcpPort;
	BOOL				autoAddCamFlag;
	UINT16				autoAddCamTcpPort;
	UINT8				pollDuration;
	UINT8				pollInterval;
	BOOL				netAcceleration;

} GENERAL_CONFIG_VER14_t;

typedef struct
{
    CHAR                    deviceNameV1[MAX_DEVICE_NAME_WIDTH_V1];     // Device Name of the NVR module
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
    CHAR                    autoConfigStartIp[IPV4_ADDR_LEN_MAX];       // auto config start ip
    CHAR                    autoConfigEndIp[IPV4_ADDR_LEN_MAX];         // auto config end ips
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
    CHAR                    devInitServerIpAddr[IPV4_ADDR_LEN_MAX];     // samas Server Ip addr
    UINT16                  devInitServerPort;                          // samas Server Port
    BOOL                    autoRecordFailFlag;
    UINT16                  videoPopupDuration;
    UINT16                  preVideoLossDuration;
    CHAR                    userNameV1[MAX_USERNAME_WIDTH_V1];
    CHAR                    passwordV1[MAX_PASSWORD_WIDTH_V1];
    BOOL                    startLiveView;                              // For Decoding in Local Client
    UINT16                  forwardedTcpPort;
    BOOL                    autoAddCamFlag;
    UINT16                  autoAddCamTcpPort;
    UINT8                   pollDuration;
    UINT8                   pollInterval;
    BOOL                    netAcceleration;
    CHAR                    deviceName[MAX_DEVICE_NAME_WIDTH];
    CHAR                    userName[MAX_USERNAME_WIDTH];
    CHAR                    password[MAX_PASSWORD_WIDTH];

} GENERAL_CONFIG_VER15_t;

/** ********** DATE AND TIME SETTINGS ********** **/
typedef struct
{
    UINT8                   timezone;                               // universal time zone
    DATE_TIME_UPDATE_MODE_e updateMode;                             // Mode of time update
    NTP_PARAMETER_V1_t      ntpV1;                                  // NTP Parameters, <server and update time interval>
    BOOL                    autoUpdateRegional;                     // Auto-update Regional
} DATE_TIME_CONFIG_VER2_t;

typedef struct
{
    UINT8                   timezone;                               // universal time zone
    DATE_TIME_UPDATE_MODE_e updateMode;                             // Mode of time update
    NTP_PARAMETER_V1_t      ntpV1;                                  // NTP Parameters, <server and update time interval>
    BOOL                    autoUpdateRegional;                     // Auto-update Regional
    NTP_PARAMETER_t         ntp;                                    // NTP Parameters, <server and update time interval> // Note : UTF-8 change
} DATE_TIME_CONFIG_VER3_t;                                          // UTF-8 Multilanguage

typedef struct
{
    UINT8                   timezone;                               // universal time zone
    DATE_TIME_UPDATE_MODE_e updateMode;                             // Mode of time update
    NTP_PARAMETER_V1_t      ntpV1;                                  // NTP Parameters, <server and update time interval>
    BOOL                    autoUpdateRegional;                     // Auto-update Regional
    NTP_PARAMETER_t         ntp;                                    // NTP Parameters, <server and update time interval> // Note : UTF-8 change
    BOOL                    syncTimeZoneToOnvifCam;                 // Auto Sync Time-zone(ONVIF)
} DATE_TIME_CONFIG_VER4_t;                                          // UTF-8 Multilanguage

/** ********** LAN SETTINGS ********** **/
typedef struct
{
    IPV4_ASSIGN_MODE_e          ipAssignMode;                       // ip assignment options
    IPV4_NETWORK_PARAMETER_t    lan;                                // network parameter for lan1, <ip, subnet mask, gateway>
    PPPOE_PARAMETER_V1_t        pppoe;                              // PPPOE parameters, <username, password>
    DNS_PARAMETER_V1_t          dns;

}LAN_CONFIG_VER2_t;

typedef struct
{
    IPV4_ASSIGN_MODE_e          ipAssignMode;                       // ip assignment options
    IPV4_NETWORK_PARAMETER_t    lan;                                // network parameter for lan1, <ip, subnet mask, gateway>
    PPPOE_PARAMETER_V1_t        pppoeV1;                            // PPPOE parameters, <username, password> // Note : UTF-8 change
    DNS_PARAMETER_V1_t          dns;
    PPPOE_PARAMETER_V2_t        pppoeV2;                            // PPPOE parameters, <username, password> //NOTE : Change size of array
} LAN_CONFIG_VER3_t;

typedef struct
{
    IPV4_ASSIGN_MODE_e          ipAssignMode;                       // ip assignment options
    IPV4_NETWORK_PARAMETER_t    lan;                                // network parameter for lan1, <ip, subnet mask, gateway>
    DNS_PARAMETER_V1_t          dns;
    PPPOE_PARAMETER_t           pppoe;                              // PPPOE parameters, <username, password>

} LAN_CONFIG_VER4_t;

/** ********** STATIC ROUTING SETTINGS ********** **/
typedef struct
{
    NETWORK_PORT_e      defaultPort;                                // default Routing Port
    ROUTING_ENTRY_V1_t  entry[MAX_STATIC_ROUTING_ENTRY];

} STATIC_ROUTING_CONFIG_VER3_t;

/** ********** IP FILTER SETTINGS ********** **/
typedef struct
{
    BOOL                        ipFilter;                           // control to Enable/Disable IP filter
    IP_ADDRESS_FILTER_MODE_e    mode;                               // one of the available filtering option
    IP_ADDRESS_RANGE_V1_t       filter[MAX_IP_FILTER];              // specifies IP address range

} IP_FILTER_CONFIG_VER1_t;

/** ********** DDNS SETTINGS ********** **/
typedef struct
{
    BOOL            ddns;                                           // control to enable/disable DDNS service
    DDNS_SERVER_e   server;                                         // DDNS server
    CHAR            username[MAX_DDNS_USERNAME_WIDTH_V1];           // username for authentication at DNS server
    CHAR            password[MAX_DDNS_PASSWORD_WIDTH_V1];           // password for authentication at DNS server
    CHAR            hostname[MAX_DDNS_HOSTNAME_WIDTH_V1];           // hostname to be updated at DNS server
    UINT8           updateDuration;                                 // update duration for DNS, in minutes

}DDNS_CONFIG_VER1_t;

/** ********** EMAIL (SMTP) SETTINGS ********** **/
typedef struct
{
    BOOL    smtp;													// control bit to ON/OFF email service
    CHAR    server[MAX_SMTP_SERVER_NAME_WIDTH_V1];					// specifies SMTP server name
    UINT16  serverPort;                                             // specifies SMTP port number
    CHAR    username[MAX_SMTP_USERNAME_WIDTH_V1];                   // username for authentication
    CHAR    password[MAX_SMTP_PASSWORD_WIDTH_V1];                   // password for authentication
    CHAR    senderAddress[MAX_SMTP_EMAILID_WIDTH_V1];               // sender's email address
    BOOL    ssl;													// control to Enable/Disable SSL feature

}SMTP_CONFIG_VER1_t;

/** ********** FTP UPLOAD SETTINGS ********** **/
typedef struct
{
    BOOL    ftp;													// control to enable/disable FTP service
    CHAR    server[MAX_FTP_SERVER_NAME_WIDTH_V1];                   // specifies FTP server address
    UINT16  serverPort;                                             // specifies FTP server port, 21
    CHAR    username[MAX_FTP_USERNAME_WIDTH_V1];                    // username for authentication at FTP server
    CHAR    password[MAX_FTP_PASSWORD_WIDTH_V1];                    // password for authentication at FTP server
    CHAR    uploadPath[MAX_FTP_UPLOAD_PATH_WIDTH_V1];               // specifies the upload path for file

}FTP_UPLOAD_CONFIG_VER1_t;

/** ********** TCP NOTIFY SETTINGS ********** **/
typedef struct
{
    BOOL    tcpNotify;                                              // control bit to ON/OFF TCP service, default is disable <0>
    CHAR    server[MAX_TCP_SERVER_NAME_WIDTH_V1];                   // specifies TCP server Address (IP or host name)
    UINT16  serverPort;                                             // specifies TCP server Port address, default is 80

}TCP_NOTIFY_CONFIG_VER1_t;

/** ********** FILE ACCESS SERVICE ********** **/
typedef struct
{
    BOOL    smbAccess;												// control enable/disable SMB/CIFS access
    BOOL    ftpAccess;												// control enable/disable FTP access
    UINT16  ftpport;												// specifies port number

}FILE_ACCESS_CONFIG_VER1_t;

/** ********** Matrix DNS Server Settings ********** **/
typedef struct
{
	BOOL 	enMacClient;
	CHAR	hostName[MAX_MATRIX_DNS_SERVER_HOST_NAME_LEN_V1];
	UINT16	forwardedPort;

}MATRIX_DNS_SERVER_CONFIG_VER1_t;

/** ********** USER ACCOUNT MANAGEMENT ********** **/
// Added Preferred Language
typedef struct
{
    CHAR 				username[MAX_USER_ACCOUNT_USERNAME_WIDTH_VER1];         // user name for account
    USER_GROUP_e 		userGroup;                                              // group number which user belongs to
    CHAR 				password[MAX_USER_ACCOUNT_PASSWORD_WIDTH_VER1];         // password for authentication
    BOOL 				userStatus;                                             // User Enabled/ disabled
    BOOL 				multiLogin;                                             // User allowed to have multiple-login
    UINT16 				loginSessionDuration;                                   // User Login Limit Duration
    USER_PRIVILEGE_u 	userPrivilege[CAMERA_CONFIG_MAX_V1];                    // camera privilege for user
	BOOL 				syncNetDev;

} USER_ACCOUNT_CONFIG_VER8_t;

// Supported user increased to 128 (V9) and Push notification rights flag added (V10)
typedef struct
{
    CHAR 				username[MAX_USER_ACCOUNT_USERNAME_WIDTH];              // user name for account
    USER_GROUP_e 		userGroup;                                              // group number which user belongs to
    CHAR 				password[MAX_USER_ACCOUNT_PASSWORD_WIDTH];              // password for authentication
    BOOL 				userStatus;                                             // User Enabled/ disabled
    BOOL 				multiLogin;                                             // User allowed to have multiple-login
    UINT16 				loginSessionDuration;                                   // User Login Limit Duration
    USER_PRIVILEGE_u 	userPrivilege[CAMERA_CONFIG_MAX_V1];                    // camera privilege for user
    BOOL 				syncNetDev;
    CHAR				preferredLanguage[MAX_LANGUAGE_FILE_NAME_LEN];

} USER_ACCOUNT_CONFIG_VER9_10_t;

typedef struct
{
    CHAR                username[MAX_USER_ACCOUNT_USERNAME_WIDTH];              // user name for account
    USER_GROUP_e        userGroup;                                              // group number which user belongs to
    CHAR                password[MAX_USER_ACCOUNT_PASSWORD_WIDTH];              // password for authentication
    BOOL                userStatus;                                             // User Enabled/ disabled
    BOOL                multiLogin;                                             // User allowed to have multiple-login
    UINT16              loginSessionDuration;                                   // User Login Limit Duration
    USER_PRIVILEGE_u    userPrivilege[CAMERA_CONFIG_MAX_V1];                    // camera privilege for user
    BOOL                syncNetDev;
    CHAR                preferredLanguage[MAX_LANGUAGE_FILE_NAME_LEN];
    BOOL                managePushNotificationRights;

} USER_ACCOUNT_CONFIG_VER11_t; // 96 channel NVR support (camera config changed from 64 to 96)

/** ********** CAMERA SETTINGS ********** **/
typedef struct
{
    BOOL                    camera;                                 // control to enable/disable camera
    CHAR                    name[MAX_CAMERA_NAME_WIDTH_V1];         // user defined name of camera
    CAMERA_TYPE_e           type;
    BOOL                    logMotionEvents;                        // whether to log motion events of the respective camera in the event log or not
    UINT8                   motionReDetectionDelay;                 // (in seconds) On detection of Motion, maintain motion detection event till time
    OSD_POS_e               cameraNamePos;                          // where to show camera name
    OSD_POS_e               statusPos;                              // where to show status
    BOOL                    dateTimeOverlay;                        // dateTimeOverlay is enable or not
    OSD_POS_e               dateTimePos;                            // position for date-time
    BOOL                    channelNameOverlay;                     // channelNameOverlay is enable or not
    CHAR                    channelName[MAX_CHANNEL_NAME_WIDTH_V1]; // channel name
    OSD_POS_e               channelNamePos;                         // position for channel name
    CHAR                    mobileNum[MAX_MOBILE_NUM_WIDTH];        // user defined name of camera
    BOOL                    privacyMaskStaus;                       // enabled / disabled
    PRIVACY_MASK_CONFIG_t   privacyMask[MAX_PRIVACY_MASKS_V5];
    BOOL                    motionDetectionStatus;                  // enabled / disabled
    MOTION_POINT_AREA_t     motionArea[MOTION_POINT_AREA_MAX];
    VIDEO_TYPE_e            recordingStream;

} CAMERA_CONFIG_VER3_t;

typedef struct
{
    BOOL                    camera;                                  // control to enable/disable camera
    CHAR                    nameV1[MAX_CAMERA_NAME_WIDTH_V1];        // user defined name of camera
    CAMERA_TYPE_e           type;
    BOOL                    logMotionEvents;                         // whether to log motion events of the respective camera in the event log or not
    UINT8                   motionReDetectionDelay;                  // (in seconds) On detection of Motion, maintain motion detection event till time
    OSD_POS_e               cameraNamePos;                           // where to show camera name
    OSD_POS_e               statusPos;                               // where to show status
    BOOL                    dateTimeOverlay;                         // dateTimeOverlay is enable or not
    OSD_POS_e               dateTimePos;                             // position for date-time
    BOOL                    channelNameOverlay;                      // channelNameOverlay is enable or not
    CHAR                    channelNameV1[MAX_CHANNEL_NAME_WIDTH_V1];// channel name
    OSD_POS_e               channelNamePos;                          // position for channel name
    CHAR                    mobileNum[MAX_MOBILE_NUM_WIDTH];         // user defined name of camera
    BOOL                    privacyMaskStaus;                        // enabled / disabled
    PRIVACY_MASK_CONFIG_t   privacyMask[MAX_PRIVACY_MASKS_V5];
    BOOL                    motionDetectionStatus;                   // enabled / disabled
    MOTION_POINT_AREA_t     motionArea[MOTION_POINT_AREA_MAX];
    VIDEO_TYPE_e            recordingStream;
    CHAR                    name[MAX_CAMERA_NAME_WIDTH];             // user defined name of camera
    CHAR                    channelName[MAX_CHANNEL_NAME_WIDTH];     // channel name

} CAMERA_CONFIG_VER4_t; // (3->4) UTF-8 mutlilanguage

typedef struct
{
    BOOL                    camera;                                     // control to enable/disable camera
    CHAR                    name[MAX_CAMERA_NAME_WIDTH];                // user defined name of camera
    CAMERA_TYPE_e           type;
    BOOL                    logMotionEvents;                            // whether to log motion events of the respective camera in the event log or not
    UINT8                   motionReDetectionDelay;                     // (in seconds) On detection of Motion, maintain motion detection event till time
    OSD_POS_e               cameraNamePos;                              // where to show camera name
    OSD_POS_e               statusPos;                                  // where to show status
    BOOL                    dateTimeOverlay;                            // dateTimeOverlay is enable or not
    OSD_POS_e               dateTimePos;                                // position for date-time
    BOOL                    channelNameOverlay;                         // channelNameOverlay is enable or not
    CHAR                    channelName[MAX_CHANNEL_NAME_WIDTH];        // channel name
    OSD_POS_e               channelNamePos;                             // position for channel name
    CHAR                    mobileNum[MAX_MOBILE_NUM_WIDTH];            // user defined name of camera
    BOOL                    privacyMaskStaus;                           // enabled / disabled
    PRIVACY_MASK_CONFIG_t   privacyMask[MAX_PRIVACY_MASKS_V5];
    BOOL                    motionDetectionStatus;                      // enabled / disabled
    MOTION_POINT_AREA_t     motionArea[MOTION_POINT_AREA_MAX];
    CHAR                    motionAreaBlockBits[MOTION_BLOCK_AREA_MAX_V7];
    VIDEO_TYPE_e            recordingStream;

} CAMERA_CONFIG_VER5_t; // (4->5) Motion area blocks are added

/** ********** SCHEDULE RECORD SETTINGS ********** **/
typedef struct
{
    BOOL scheduleRecording;                                         // control to turn ON/OFF schedule recording
    DAY_RECORD_SCHEDULE_t dailyRecord[MAX_WEEK_DAYS];               // weekly recording schedule for a camera

}SCHEDULE_RECORD_CONFIG_VER1_t;

/** ********** PTZ PRESET POSITIONS SETTINGS ********** **/
typedef struct
{
    CHAR name[MAX_PRESET_POSITION][MAX_PRESET_POSITION_NAME_WIDTH_VER1];    // name of the respective PTZ position

}PTZ_PRESET_CONFIG_VER1_t;

typedef struct
{
    CHAR name[MAX_PRESET_POSITION][MAX_PRESET_POSITION_NAME_WIDTH]; // name of the respective PTZ position

} PTZ_PRESET_CONFIG_VER2_t; // (1->2) UTF-8 mutlilanguage

/** ********** PRESET TOUR SETTINGS ********** **/
typedef struct
{
    CHAR name[MAX_TOUR_NAME_WIDTH_VER1];                            // user defined name of the tour
    TOUR_PATTERN_e tourPattern;                                     // order of sequence for tour
    UINT8 pauseBetweenTour;                                         // time to pause between two consecutive tours
    ORDER_NUMBER_t ptz[MAX_ORDER_COUNT];                            // sequential order of the preset positions for particular tour

}TOUR_VER1_t;

typedef struct
{
    BOOL        manualTourOverride;                                 // control bit to turn ON/OFF Manual override over schedule tour
    UINT8       manualTour;                                         // number of tour to be initiated manually
    TOUR_VER1_t tour[MAX_TOUR_NUMBER];                              // tours settings
    BOOL        activeTourOverride;                                 // overide active tour by Ptz Control

} PRESET_TOUR_CONFIG_VER3_t;

/** ********** SYSTEM SENSOR INPUT SETTINGS ********** **/
typedef struct
{
    BOOL            sensorDetect;                                   // control bit to turn ON/OFF detection of the sensor
    CHAR            name[MAX_SENSOR_NAME_WIDTH_OLD];                // user defined name of the sensor
    SENSOR_MODE_e   normalMode;                                     // normal mode option [NO- normally open / NC- normally close]
    UINT8           debounceTime;                                   // debounce time in seconds

}SENSOR_CONFIG_VER1_t;

/** ********** SYSTEM ALARM OUTPUT SETTINGS ********** **/
typedef struct
{
    BOOL            alarmOutput;                                    // control bit to turn ON/OFF alarm detection
    CHAR            name[MAX_ALARM_NAME_WIDTH_OLD];                 // user defined alarm name
    ALARM_MODE_e    activeMode;                                     // alarm activation mode, event based or time period based
    UINT8           pulsePeriod;                                    // time period in seconds for which alarm should be keep active

}ALARM_CONFIG_VER1_t;

/** ********** UPLOAD IMAGE SETTINGS ********** **/
typedef struct
{
    CHAR emailAddress[MAX_EMAIL_ADDRESS_WIDTH_VER1];                // mail address to send the images to
    CHAR subject[MAX_EMAIL_SUBJECT_WIDTH_VER1];                     // subject of the mail
    CHAR message[MAX_EMAIL_MESSAGE_WIDTH_VER1];                     // message of the mail

}EMAIL_PARAMETER_VER1_t;

typedef struct
{
    CHAR                    resolution[MAX_RESOLUTION_NAME_LEN];    // upload image resolution
    UPLOAD_LOCATION_e       uploadLocation;                         // image upload location
    EMAIL_PARAMETER_VER1_t 	uploadToEmail;                          // email parameter to upload the image
    UINT8                   uploadImageRate;                        // upload rate in minutes

}IMAGE_UPLOAD_CONFIG_VER2_t;

/** ********** STORAGE SETTINGS ********** **/
typedef struct
{
    HDD_FULL_t              hddFull;                            // parameters for HDD full event
    BOOL                    recordRetentionStatus;
    RECORD_RETENTION_TYPE_e recordRetentionType;
    UINT8                   driveWiseRecordCleanupByDays;       // number of days, in case of "older then days" cleanup option
    STORAGE_ALERT_t         hddStorage;                         // parameters for storage alert
    BOOL                    backUpRetentionStatus;
    UINT16                  cameraWiseRecordCleanupByDays[CAMERA_CONFIG_MAX_V1];
    UINT16                  cameraWiseBackUpCleanupByDays[CAMERA_CONFIG_MAX_V1];

} STORAGE_CONFIG_VER2_t; // 96 channel NVR support (camera config changed from 64 to 96)

/** ********** SCHEDULE BACKUP SETTINGS ********** **/
typedef struct
{
    BOOL                    scheduleBackup;                     // Flag to indicate statsu of schedule backup
    SB_MODE_e               mode;                               // schedule backup mode, hourly, daily, weekly
    SB_EVERY_HOUR_MODE_e    everyHourMode;                      // schedule back every hour mode
    SB_EVERYDAY_CONFIG_t    everydayBackup;                     // daily backup schedule
    SB_WEEKLY_CONFIG_t      weeklyBackup;                       // weekly backup schedule
    UINT64                  camera;                             // Bit-Wise flags to indicate if schedule back-up is enabled /disabled for particular camera
    SB_LOCATION_e           backupLocation;                     // backup location

} SCHEDULE_BACKUP_CONFIG_VER3_t; // 96 channel NVR support (camera config changed from 64 to 96)

/** ********** MANUAL BACKUP SETTINGS ********** **/
typedef struct
{
    BOOL 				manualBackup;                           // Flag to indicate status of manual backup
    MB_DURATION_e 		duration;                               // backup duration
    UINT64              camera;                                 // Bit-Wise flags to indicate if
    MB_LOCATION_e 		backupLocation;                         // backup location

} MANUAL_BACKUP_CONFIG_VER4_t; // 96 channel NVR support (camera config changed from 64 to 96)

/** ********** EVENT AND ACTION MANAGEMENT ********** **/
#define MAX_CAMERA_EVENT_VER4   (14)
#define MAX_CAMERA_EVENT_VER7   (16)

#define MAX_SYSTEM_EVENT_VER5   (7)                       // (4->5) smslane v2 support added


typedef struct
{
    CHAR mobileNumber1[MAX_MOBILE_NUMBER_WIDTH];        // mobile number 1
    CHAR mobileNumber2[MAX_MOBILE_NUMBER_WIDTH];        // mobile number 2
    CHAR message[MAX_SMS_WIDTH_VER1];                   // message to send

}SMS_PARAMTER_VER1_t;

typedef struct
{
    CHAR mobileNumber1[MAX_MOBILE_NUMBER_WIDTH];        // mobile number 1
    CHAR mobileNumber2[MAX_MOBILE_NUMBER_WIDTH];        // mobile number 2
    CHAR message[MAX_SMS_WIDTH_VER2];                   // message to send

}SMS_PARAMTER_VER2_t; // new size as per UTF-8 multilanguage

typedef struct
{
    BOOL                        alarmRecord[CAMERA_CONFIG_MAX_V1];          // select cameras to record stream on occurrence of event
    BOOL                        uploadImage[CAMERA_CONFIG_MAX_V1];          // select cameras to upload images from on occurrence of event
    EMAIL_PARAMETER_VER1_t      sendEmail;                                  // email parameters to send notifications
    CHAR                        sendTcp[MAX_TCP_MESSAGE_WIDTH_V1];          // TCP notification message
    SMS_PARAMTER_VER1_t         smsParameter;								// SMS parameters for notification
    GOTO_PRESET_POSITION_t      gotoPosition;                               // preset position to set against an event
    BOOL                        systemAlarmOutput[MAX_ALARM];               // control bit to enable system alarm output
    CAMERA_ALARM_PARAMETER_t    cameraAlarmOutput;                          // camera alarm parameters

}ACTION_PARAMETERS_VER1_t;

typedef struct
{
    BOOL                        alarmRecord[CAMERA_CONFIG_MAX_V1];          // select cameras to record stream on occurrence of event
    BOOL                        uploadImage[CAMERA_CONFIG_MAX_V1];          // select cameras to upload images from on occurrence of event
    EMAIL_PARAMETER_t           sendEmail;                                  // email parameters to send notifications
    CHAR                        sendTcp[MAX_TCP_MESSAGE_WIDTH];             // TCP notification message
    SMS_PARAMTER_VER2_t         smsParameter;                               // SMS parameters for notification
    GOTO_PRESET_POSITION_t      gotoPosition;                               // preset position to set against an event
    BOOL                        systemAlarmOutput[MAX_ALARM];               // control bit to enable system alarm output
    CAMERA_ALARM_PARAMETER_t    cameraAlarmOutput;                          // camera alarm parameters

}ACTION_PARAMETERS_VER2_t; // new size as per UTF-8 multilanguage

typedef struct
{
    BOOL                        alarmRecord[CAMERA_CONFIG_MAX_V1];      // select cameras to record stream on occurrence of event
    BOOL                        uploadImage[CAMERA_CONFIG_MAX_V1];      // select cameras to upload images from on occurrence of event
    EMAIL_PARAMETER_t           sendEmail;                              // email parameters to send notifications
    CHAR                        sendTcp[MAX_TCP_MESSAGE_WIDTH];         // TCP notification message
    SMS_PARAMTER_t              smsParameter;                           // SMS parameters for notification
    GOTO_PRESET_POSITION_t      gotoPosition;                           // preset position to set against an event
    BOOL                        systemAlarmOutput[MAX_ALARM];           // control bit to enable system alarm output
    CAMERA_ALARM_PARAMETER_t    cameraAlarmOutput;                      // camera alarm parameters

}ACTION_PARAMETERS_VER3_t; // 96 channel NVR support (camera config changed from 64 to 96)

/** ********** CAMERA EVENT AND ACTION SETTINGS ********** **/

typedef struct
{
    BOOL                        action;                         // control bit to ON/OFF action against an event
    ACTION_PARAMETERS_VER1_t    actionParam;
    WEEKLY_ACTION_SCHEDULE_t    weeklySchedule[MAX_WEEK_DAYS];  // weekly action schedule <time and action list>
    UINT64                      copyToCam;

} CAMERA_EVENT_CONFIG_VER4_5_t; // ACTION_PARAMETER_t size changed: UTF-8 MultiLanguage

typedef struct
{
    BOOL                        action;                         // control bit to ON/OFF action against an event
    ACTION_PARAMETERS_VER2_t    actionParam;
    WEEKLY_ACTION_SCHEDULE_t    weeklySchedule[MAX_WEEK_DAYS];  // weekly action schedule <time and action list>
    UINT64                      copyToCam;

} CAMERA_EVENT_CONFIG_VER6_t; // ACTION_PARAMETER_t size changed: smslane v2 support added

typedef struct
{
    BOOL                        action;                         // control bit to ON/OFF action against an event
    ACTION_PARAMETERS_VER3_t    actionParam;
    WEEKLY_ACTION_SCHEDULE_t    weeklySchedule[MAX_WEEK_DAYS];  // weekly action schedule <time and action list>
    UINT64                      copyToCam;

} CAMERA_EVENT_CONFIG_VER7_8_t; // ACTION_PARAMETER_t size changed: No motion event is added (V7) & 96 channel NVR support added (V8)

/** ********** SENSOR EVENT AND ACTION SETTINGS ********** **/

typedef struct
{
    BOOL                        action;                         // control bit to ON/OFF action against an event
    ACTION_PARAMETERS_VER1_t    actionParam;
    WEEKLY_ACTION_SCHEDULE_t    weeklySchedule[MAX_WEEK_DAYS];  // weekly action schedule <time and action list>

}SENSOR_EVENT_CONFIG_VER2_t; // ACTION_PARAMETER_t size changed: UTF-8 MultiLanguage

typedef struct
{
    BOOL                        action;                         // control bit to ON/OFF action against an event
    ACTION_PARAMETERS_VER2_t    actionParam;
    WEEKLY_ACTION_SCHEDULE_t    weeklySchedule[MAX_WEEK_DAYS];  // weekly action schedule <time and action list>

} SENSOR_EVENT_CONFIG_VER3_t; // ACTION_PARAMETER_t size changed: smslane v2 support added

typedef struct
{
    BOOL                        action;                         // control bit to ON/OFF action against an event
    ACTION_PARAMETERS_VER3_t    actionParam;
    WEEKLY_ACTION_SCHEDULE_t    weeklySchedule[MAX_WEEK_DAYS];  // weekly action schedule <time and action list>

} SENSOR_EVENT_CONFIG_VER4_t; // ACTION_PARAMETER_t size changed: 96 channel NVR support added

/** ********** SYSTEM EVENT AND ACTION SETTINGS ********** **/

typedef struct
{
    BOOL                        action;                         // control bit to ON/OFF action against an event
    ACTION_PARAMETERS_VER1_t    actionParam;
    ACTION_BIT_u                actionBits;                     // permanent action control bits

} SYSTEM_EVENT_CONFIG_VER3_t; // ACTION_PARAMETER_t size changed: UTF-8 MultiLanguage

typedef struct
{
    BOOL                        action;                         // control bit to ON/OFF action against an event
    ACTION_PARAMETERS_VER2_t    actionParam;
    ACTION_BIT_u                actionBits;                     // permanent action control bits

} SYSTEM_EVENT_CONFIG_VER4_t; // ACTION_PARAMETER_t size changed: smslane v2 support added

typedef struct
{
    BOOL                        action;
    ACTION_PARAMETERS_VER3_t    actionParam;
    ACTION_BIT_u                actionBits;

} SYSTEM_EVENT_CONFIG_VER5_6_t; // Firmware upgrade event and action added (V5) & ACTION_PARAMETER_t size changed: 96 channel NVR support added (V6)

/** ********** MOBILE BROADBAND SETTINGS ********** **/
typedef struct
{
    CHAR					profileName[BROADBAND_PROFILE_NAME_LEN_V1];
    CHAR					dialNumber[BROADBAND_DIAL_NUMBER_LEN_V1];
    CHAR					userName[BROADBAND_USERNAME_LEN_V1];
    CHAR					password[BROADBAND_PASSWORD_LEN_V1];
    MOBILE_SERVICE_TYPE_e	serviceType;
    CHAR					apn[BROADBAND_APN_LEN_V1];
}BROAD_BAND_PROFILE_VER1_t;

typedef struct
{
    CHAR					profileName[BROADBAND_PROFILE_NAME_LEN];
    CHAR					dialNumber[BROADBAND_DIAL_NUMBER_LEN];
    CHAR					userName[BROADBAND_USERNAME_LEN];
    CHAR					password[BROADBAND_PASSWORD_LEN];
    MOBILE_SERVICE_TYPE_e	serviceType;
    CHAR					apn[BROADBAND_APN_LEN];
}BROAD_BAND_PROFILE_VER2_t; // (2->3) UTF-8 MultiLanguage

typedef struct
{
    UINT8                       activeProfile;
    BROAD_BAND_PROFILE_VER1_t	broadBandCfg[MAX_BROADBAND_PROFILE];
}BROAD_BAND_CONFIG_VER2_t;

typedef struct
{
    UINT8                       activeProfile;
    BROAD_BAND_PROFILE_VER2_t	broadBandCfg[MAX_BROADBAND_PROFILE];
}BROAD_BAND_CONFIG_VER3_t;

/** ************ SMS SETTINGS ********** **/
typedef struct
{
	SMS_MODE_e		mode;
	SMS_SERVER_e	smsServer;
	CHAR			userName[MAX_SMS_ACC_USERNAME_WIDTH_V1];
	CHAR			password[MAX_SMS_ACC_PASSWORD_WIDTH_V1];
	CHAR 			sendorId[MAX_SMS_ACC_SENDOR_ID_WIDTH_V1];
	BOOL			sendAsFlash;

}SMS_CONFIG_VER1_t;

typedef struct
{
    SMS_MODE_e      mode;
    SMS_SERVER_e    smsServer;
    CHAR            userName[MAX_SMS_ACC_USERNAME_WIDTH_V2];
    CHAR            password[MAX_SMS_ACC_PASSWORD_WIDTH_V2];
    CHAR            sendorId[MAX_SMS_ACC_SENDOR_ID_WIDTH];
    BOOL            sendAsFlash;

} SMS_CONFIG_VER2_t;

/** ********** Network Drive Settings ********** **/
typedef struct
{
    BOOL            enable;
    CHAR            name[MAX_NW_DRIVE_NAME_LEN_V1];
    CHAR            ipAddr[IPV4_ADDR_LEN_MAX];
    CHAR            userName[MAX_NW_DRIVE_USER_NAME_LEN_V1];
    CHAR            password[MAX_NW_DRIVE_PASSWORD_LEN_V1];
    FILE_SYS_TYPE_e fileSys;
    CHAR            dfltFolder[MAX_NW_DRIVE_DFLT_FOLDER_PATH_LEN_V1];

} NETWORK_DRIVE_CONFIG_VER2_t; // UTF-8 MultiLanguage

typedef struct
{
    BOOL 				enable;
    CHAR 				name[MAX_NW_DRIVE_NAME_LEN];
    CHAR 				ipAddr[IPV4_ADDR_LEN_MAX];
    CHAR 				userName[MAX_NW_DRIVE_USER_NAME_LEN];
    CHAR 				password[MAX_NW_DRIVE_PASSWORD_LEN];
    FILE_SYS_TYPE_e 	fileSys;
    CHAR 				dfltFolder[MAX_NW_DRIVE_DFLT_FOLDER_PATH_LEN];

} NETWORK_DRIVE_CONFIG_VER3_t;

/** ********** IP Camera Settings ********** **/
typedef struct
{
    CHAR            brand[MAX_BRAND_NAME_LEN];                      // camera brand name
    CHAR            model[MAX_MODEL_NAME_LEN];                      // camera model name
    CHAR            url[MAX_CAMERA_URL_WIDTH];                      // url to be used for generic camera
    CHAR            cameraAddress[MAX_CAMERA_ADDRESS_WIDTH_VER1];   // ip address of camera
    UINT16          httpPort;                                       // http port of camera
    UINT16          rtspPort;                                       // rtsp port number of camera
    BOOL            accessThroughNat;                               // Flag indicating whether Camera access through NAT
    CHAR            routerAddress[MAX_ROUTER_ADDRESS_WIDTH_VER1];   // ip address of the router when camera behind NAT
    UINT16          forwardedHttpPort;                              // mapped http port number of camera in the router
    UINT16          forawrdedRtspPort;                              // mapped rtsp port number of camera in the router
    CHAR            username[MAX_CAMERA_USERNAME_WIDTH_VER4];       // user name for authentication of camera
    CHAR            password[MAX_CAMERA_PASSWORD_WIDTH_VER4];       // password for authentication of camera
    BOOL            onvifSupportF;                                  // Camera is ONVIF supported
    UINT16          onvifPort;                                      // ONVIF port
    UINT16          forwardedOnvifPort;                             // mapped ONVIF port
    RTSP_PROTOCOL_e rtspProtocol;                                   // RTSP PROTOCOL

}IP_CAMERA_CONFIG_VER4_t; /* Added RTSP Protocol */

typedef struct
{
    CHAR            brand[MAX_BRAND_NAME_LEN];                      // camera brand name
    CHAR            model[MAX_MODEL_NAME_LEN];                      // camera model name
    CHAR            url[MAX_CAMERA_URL_WIDTH];                      // url to be used for generic camera
    CHAR            cameraAddress[MAX_CAMERA_ADDRESS_WIDTH_VER1];   // ip address of camera
    UINT16          httpPort;                                       // http port of camera
    UINT16          rtspPort;                                       // rtsp port number of camera
    BOOL            accessThroughNat;                               // Flag indicating whether Camera access through NAT
    CHAR            routerAddress[MAX_ROUTER_ADDRESS_WIDTH_VER1];   // ip address of the router when camera behind NAT
    UINT16          forwardedHttpPort;                              // mapped http port number of camera in the router
    UINT16          forawrdedRtspPort;                              // mapped rtsp port number of camera in the router
    CHAR            username[MAX_CAMERA_USERNAME_WIDTH_VER4];       // user name for authentication of camera
    CHAR            password[MAX_CAMERA_PASSWORD_WIDTH_VER4];       // password for authentication of camera
    BOOL            onvifSupportF;                                  // Camera is ONVIF supported
    UINT16          onvifPort;                                      // ONVIF port
    UINT16          forwardedOnvifPort;                             // mapped ONVIF port
    RTSP_PROTOCOL_e rtspProtocol;                                   // RTSP PROTOCOL
    CHAR            macAddr[MAX_MAC_ADDRESS_WIDTH];

} IP_CAMERA_CONFIG_VER5_t; /* Added MAC-address */

typedef struct
{
    CHAR            brand[MAX_BRAND_NAME_LEN];                      // camera brand name
    CHAR            model[MAX_MODEL_NAME_LEN];                      // camera model name
    CHAR            url[MAX_CAMERA_URL_WIDTH];                      // url to be used for generic camera
    CHAR            cameraAddress[MAX_CAMERA_ADDRESS_WIDTH];        // ip address of camera
    UINT16          httpPort;                                       // http port of camera
    UINT16          rtspPort;                                       // rtsp port number of camera
    BOOL            accessThroughNat;                               // Flag indicating whether Camera access through NAT
    CHAR            routerAddress[MAX_ROUTER_ADDRESS_WIDTH];        // ip address of the router when camera behind NAT
    UINT16          forwardedHttpPort;                              // mapped http port number of camera in the router
    UINT16          forawrdedRtspPort;                              // mapped rtsp port number of camera in the router
    CHAR            username[MAX_CAMERA_USERNAME_WIDTH];            // user name for authentication of camera
    CHAR            password[MAX_CAMERA_PASSWORD_WIDTH_VER7];       // password for authentication of camera
    BOOL            onvifSupportF;                                  // Camera is ONVIF supported
    UINT16          onvifPort;                                      // ONVIF port
    UINT16          forwardedOnvifPort;                             // mapped ONVIF port
    RTSP_PROTOCOL_e rtspProtocol;                                   // RTSP PROTOCOL
    CHAR            macAddr[MAX_MAC_ADDRESS_WIDTH];

} IP_CAMERA_CONFIG_VER6_t; /* UTF-8 support: Multilanguage */

typedef struct
{
    CHAR            brand[MAX_BRAND_NAME_LEN];                      // camera brand name
    CHAR            model[MAX_MODEL_NAME_LEN];                      // camera model name
    CHAR            url[MAX_CAMERA_URL_WIDTH];                      // url to be used for generic camera
    CHAR            cameraAddress[MAX_CAMERA_ADDRESS_WIDTH];        // ip address of camera
    UINT16          httpPort;                                       // http port of camera
    UINT16          rtspPort;                                       // rtsp port number of camera
    CHAR            username[MAX_CAMERA_USERNAME_WIDTH];            // user name for authentication of camera
    CHAR            password[MAX_CAMERA_PASSWORD_WIDTH_VER7];       // password for authentication of camera
    BOOL            onvifSupportF;                                  // Camera is ONVIF supported
    UINT16          onvifPort;                                      // ONVIF port
    RTSP_PROTOCOL_e rtspProtocol;                                   // RTSP PROTOCOL
    CHAR            macAddr[MAX_MAC_ADDRESS_WIDTH];

} IP_CAMERA_CONFIG_VER7_t;  /* Password length increased from 16 to 20 chars */

typedef struct
{
    CHAR            brand[MAX_BRAND_NAME_LEN];                      // camera brand name
    CHAR            model[MAX_MODEL_NAME_LEN];                      // camera model name
    CHAR            url[MAX_CAMERA_URL_WIDTH];                      // url to be used for generic camera
    CHAR            cameraAddress[MAX_CAMERA_ADDRESS_WIDTH];        // ip address of camera
    UINT16          httpPort;                                       // http port of camera
    UINT16          rtspPort;                                       // rtsp port number of camera
    CHAR            username[MAX_CAMERA_USERNAME_WIDTH];            // user name for authentication of camera
    CHAR            password[MAX_CAMERA_PASSWORD_WIDTH];            // password for authentication of camera
    BOOL            onvifSupportF;                                  // Camera is ONVIF supported
    UINT16          onvifPort;                                      // ONVIF port
    RTSP_PROTOCOL_e rtspProtocol;                                   // RTSP PROTOCOL
    CHAR            macAddr[MAX_MAC_ADDRESS_WIDTH];

} IP_CAMERA_CONFIG_VER8_t;  /* Matrix camera model name correction */

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

} IP_CAMERA_CONFIG_VER9_t;  /* Axis camera support removal */

/** ********** NETWORK_DEVICE_CONFIG_t ********** **/
typedef struct
{
    CHAR 				deviceName[MAX_NETWORK_DEVICE_NAME_LEN_V1];             // network device name
    REGISTER_MODE_e 	registerMode;                                           // network register mode for connect to the network device
    CHAR 				registerModeAddress[MAX_REGISTER_MODE_ADDRESS_LEN_V1];  // register Mode address of network device
    UINT16 				port;                                                   // port address of network device
    CHAR 				username[MAX_NETWORK_DEVICE_USERNAME_LEN_V1];           // username of network device
    CHAR 				password[MAX_NETWORK_DEVICE_PASSWORD_LEN_V1];           // password of network device
    BOOL 				enable;                                                 // network device enable / disable
    BOOL 				autoLogin;                                              // auto login to network device enable / disable
    LIVE_STREAM_TYPE_e  liveStreamType;                                         // live stream type
    BOOL 				preferDeviceCredential;                                 // For Prefering Native Device Credentials. Enable/Disable
	UINT16				forwardedTcpPort;
} NETWORK_DEVICE_CONFIG_VER4_t; // UTF-8 MultiLanguage

/** ********** SNAPSHOT_CONFIG_t ********** **/
typedef struct
{
	BOOL					snapShotEnable;								 // snapshot Enable
	UPLOAD_LOCATION_e 		snapShotuploadLocation;						 // image upload location
    EMAIL_PARAMETER_VER1_t 	snapShotuploadToEmail;						 // email parameter to upload the image
	UINT8 					snapShotuploadImageRate;					 // upload rate in minutes

}SNAPSHOT_CONFIG_VER1_t;

typedef struct 
{
    BOOL 				snapShotEnable;                                 // snapshot Enable
    UPLOAD_LOCATION_e 	snapShotuploadLocation;                         // image upload location
    EMAIL_PARAMETER_t 	snapShotuploadToEmail;                          // email parameter to upload the image
    UINT8 				snapShotuploadImageRate;                        // upload rate in minutes

} SNAPSHOT_CONFIG_VER2_t;

/** ********** Login Policy ********** **/
typedef struct
{
	UINT8 					minPassLength;
	PASSWORD_STRENGTH_e 	passwordStrength;
	BOOL 					passResetStatus;
	UINT16 					passResetPeriod;
	LOGIN_LOCK_STATUS_e 	lockAccountStatus;
	UINT16 					invalidLoginAttempt;
	UINT16 					autoLockTimer;

} LOGIN_POLICY_CONFIG_VER1_t;

/** ********** P2P SETTINGS ********** **/
typedef struct
{
    BOOL    p2pEnable;

} P2P_CONFIG_VER1_t; /* Fallback to relay server flag added */

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL LoadOldConfigParam(UINT8 configId, INT32 fileFd, UINT32 fileVersion);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* CONFIGURATIONOLD_H_ */
